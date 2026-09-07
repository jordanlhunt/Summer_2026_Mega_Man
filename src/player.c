#include "player.h"
#include "collision.h"
#include "config.h"
#include "entity.h"
#include "input.h"
#include "level.h"
#include <math.h>
#include <stdbool.h>
/**
 * If the bottom of the box has cross a one-wa platform (tile == TILE_ONE_WAY)
 * while moving downward, snap the box onto the top of the tile. Returns true if
 * snap has occurred
 */
static bool IsSnappedToPlatform(Entity *boundingBox, float originalY,
                                const Level *level) {
  if (boundingBox->velocityY < 0.0f) {
    return false;
  }
  float playerCollisionBoxBottom = boundingBox->y + boundingBox->height;
  float originalBottom = originalY + boundingBox->height;
  int tileX =
      (int)floorf((boundingBox->x + boundingBox->width * .5f) / TILE_SIZE);
  if (tileX < 0 || tileX >= level->width) {
    return false;
  }
  // Calculate the range of tiles the player's bottom has passed through this
  // frame
  int startTileY = (int)floorf(originalBottom / TILE_SIZE);
  int endTileY = (int)floorf(playerCollisionBoxBottom / TILE_SIZE);
  // Iterate through all tiles to prevent tunneling
  for (int tileY = startTileY; tileY <= endTileY; tileY++) {
    if (tileY < 0 || tileY >= level->height) {
      continue;
    }
    unsigned char tile = LevelGetTile(level, tileX, tileY);
    if (tile == TILE_ONE_WAY) {
      float tileTop = (float)(tileY * TILE_SIZE);
      if (playerCollisionBoxBottom >= tileTop) {
        boundingBox->y = tileTop - boundingBox->height;
        boundingBox->isOnGround = true;
        boundingBox->velocityY = 0.0f;
        return true;
      }
    }
  }
  return false;
}
static void DecreaseTimer(float *timer, float deltaTime) {
  if (*timer > 0.0f) {
    *timer -= deltaTime;
    if (*timer < 0.0f) {
      *timer = 0.0f;
    }
  }
}
/* Integrates velocity into position and resolves tile collisions on both
 * axes. Uses entity fields for position and velocity. */
static void PlayerMoveAndResolve(Player *player, const Level *level,
                                 float deltaTime) {
  float originalY = player->entity.y;
  player->entity.isOnGround = false;
  player->entity.x += player->entity.velocityX * deltaTime;
  CollisionResolveTileAxis(&player->entity, level, true);
  player->entity.y += player->entity.velocityY * deltaTime;
  CollisionResolveTileAxis(&player->entity, level, false);
  IsSnappedToPlatform(&player->entity, originalY, level);
}
static void PlayerUpdateTimers(Player *player, const Input *input,
                               float deltaTime) {
  /* ----- Cooldowns and Timers  ----------------------------------------- */
  DecreaseTimer(&player->dashCooldown, deltaTime);
  DecreaseTimer(&player->shootCooldown, deltaTime);
  DecreaseTimer(&player->jumpBufferTimer, deltaTime);
  DecreaseTimer(&player->shootAnimationTimer, deltaTime);
  if (player->entity.isOnGround) {
    player->coyoteTimer = COYOTE_TIME;
  } else {
    DecreaseTimer(&player->coyoteTimer, deltaTime);
  }
  if (input->isJumpJustPressed) {
    player->jumpBufferTimer = JUMP_BUFFER_TIME;
  }
}
/**
 * Starts a dash if conditions are met, and advances an in-progress dash.
 * Returns if the dash consumed this frame's update - the caller should return
 * immediately without running normal movement/jump/gravity
 */
static bool PlayerHandleDash(Player *player, const Input *input,
                             const Level *level, float deltaTime) {
  if (input->isDashJustPressed && player->dashCooldown <= 0.0f &&
      !player->isDashing && player->entity.isOnGround) {
    player->isDashing = true;
    player->dashTimer = DASH_DURATION;
    player->dashTimer = DASH_COOLDOWN;
    if (player->isFacingRight) {
      player->entity.velocityX = DASH_SPEED;
    } else {
      player->entity.velocityX = -DASH_SPEED;
    }
    player->entity.velocityY = 0.0f;
    player->currentPlayerState = STATE_DASHING;
  }
  if (!player->isDashing) {
    return false;
  }
  DecreaseTimer(&player->dashTimer, deltaTime);
  if (player->dashTimer <= 0.0f) {
    player->isDashing = false;
    player->entity.velocityX *= .3f;
    return false;
  }
  player->entity.velocityY += GRAVITY * deltaTime * DASH_GRAVITY_SCALE;
  if (player->entity.velocityY > MAX_FALL_SPEED) {
    player->entity.velocityY = MAX_FALL_SPEED;
  }
  PlayerMoveAndResolve(player, level, deltaTime);
  player->animationTimer += deltaTime;
  return true;
}
static void PlayerHandleGroundToAirMovement(Player *player, const Input *input,
                                            float deltaTime) {
  float targetVelocityX = 0.0f;
  if (input->moveLeft) {
    targetVelocityX = -PLAYER_SPEED;
    if (!player->isWallSliding) {
      player->isFacingRight = false;
    }
  }
  if (input->moveRight) {
    targetVelocityX = PLAYER_SPEED;
    if (!player->isWallSliding) {
      player->isFacingRight = true;
    }
  }
  float acceleration = 0.0f;
  if (player->entity.isOnGround) {
    acceleration = GROUND_ACCELERATION;
  } else {
    acceleration = AIR_ACCELERATION;
  }
  player->entity.velocityX +=
      (targetVelocityX - player->entity.velocityX) * acceleration * deltaTime;
}
static void PlayerHandleWallSlide(Player *player, const Input *input,
                                  const Level *level, float deltaTime) {
  player->isWallSliding = false;
  player->canWallJump = false;
  player->wallDirection = 0;
  if (!player->entity.isOnGround && player->entity.velocityY > 0.0f) {
    bool isWallOnLeft = CollisionCheckWallLeft(
        level, player->entity.x, player->entity.y, player->entity.height);
    bool isWallOnRight =
        CollisionCheckWallRight(level, player->entity.x, player->entity.y,
                                player->entity.width, player->entity.height);
    if (input->moveLeft && isWallOnLeft) {
      player->isWallSliding = true;
      player->wallDirection = -1;
      player->isFacingRight = true;
    } else if (input->moveRight && isWallOnRight) {
      player->isWallSliding = true;
      player->wallDirection = 1;
      player->isFacingRight = false;
    }
  }
  if (player->isWallSliding) {
    player->entity.velocityY =
        fminf(player->entity.velocityY, WALL_SLIDE_SPEED);
    player->wallSlideTimer += deltaTime;
    player->canWallJump = true;
  } else {
    player->wallSlideTimer = 0.0f;
  }
}
static void PlayerHandleJump(Player *player, const Input *input) {
  if (player->jumpBufferTimer > 0.0f &&
      (player->entity.isOnGround || player->coyoteTimer > 0.0f)) {
    player->entity.velocityY = -JUMP_FORCE;
    player->jumpBufferTimer = 0.0f;
    player->coyoteTimer = 0.0f;
    player->entity.isOnGround = false;
    player->currentPlayerState = STATE_JUMPING;
  }
  if (player->jumpBufferTimer > 0.0f && player->canWallJump) {
    player->entity.velocityY = -WALL_JUMP_FORCE_Y;
    player->entity.velocityX = -player->wallDirection * WALL_JUMP_FORCE_X;
    player->isWallSliding = false;
    player->canWallJump = false;
    player->jumpBufferTimer = 0.0f;
    player->currentPlayerState = STATE_JUMPING;
    player->isFacingRight = (player->entity.velocityX > 0.0f);
  }
  if (!input->jumpDown && player->entity.velocityY < VARIABLE_JUMP_THRESHOLD &&
      !player->isWallSliding) {
    player->entity.velocityY *= VARIABLE_JUMP_MULTIPLIER;
  }
}
static void PlayerApplyGravity(Player *player, float deltaTime) {
  if (!player->isWallSliding) {
    player->entity.velocityY += GRAVITY * deltaTime;
  } else {
    player->entity.velocityY += GRAVITY * deltaTime * WALL_SLIDE_GRAVITY_SCALE;
  }
  if (player->entity.velocityY > MAX_FALL_SPEED) {
    player->entity.velocityY = MAX_FALL_SPEED;
  }
}
static void PlayerUpdateAnimationState(Player *player,
                                       PlayerState previousPlayerState,
                                       float deltaTime) {
  if (player->isDashing) {
    player->currentPlayerState = STATE_DASHING;
  } else if (player->isWallSliding) {
    player->currentPlayerState = STATE_WALL_SLIDING;
  } else if (!player->entity.isOnGround) {
    if (player->entity.velocityY < 0) {
      player->currentPlayerState = STATE_JUMPING;
    } else {
      player->currentPlayerState = STATE_FALLING;
    }
  } else if (fabsf(player->entity.velocityX) > 10.0f) {
    player->currentPlayerState = STATE_RUNNING;
  } else {
    player->currentPlayerState = STATE_IDLE;
  }
  if (player->currentPlayerState != previousPlayerState) {
    player->animationTimer = 0.0f;
  }
  player->animationTimer += deltaTime;
}
void PlayerUpdate(Player *player, const Input *input, const Level *level,
                  float deltaTime) {
  PlayerState previousPlayerState = player->currentPlayerState;
  PlayerUpdateTimers(player, input, deltaTime);
  if (PlayerHandleDash(player, input, level, deltaTime)) {
    return;
  }
  PlayerHandleGroundToAirMovement(player, input, deltaTime);
  PlayerHandleWallSlide(player, input, level, deltaTime);
  PlayerHandleJump(player, input);
  PlayerApplyGravity(player, deltaTime);
  PlayerMoveAndResolve(player, level, deltaTime);
  PlayerUpdateAnimationState(player, previousPlayerState, deltaTime);
}