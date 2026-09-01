#include "player.h"

#include "collision.h"
#include "input.h"
#include "level.h"

/**
 * If the bottom of the box has cross a one-wa platform (tile == '2')
 * while moving downward, snap the box onto the top of the tile. Returns true if
 * snap has occurred
 */
static bool IsSnappedToPlaform(AxisAlignedBoundingBox *boundingBox,
                               float originalY, const Level *level) {
  if (boundingBox->velocityY < 0.0f) {
    return false;
  }
  float playerCollisionBoxBottom = boundingBox->y + boundingBox->height;
  int tileX =
      (int)floorf((boundingBox->x + boundingBox->width * .5f) / TILE_SIZE);
  int tileY = (int)floorf(playerCollisionBoxBottom / TILE_SIZE);
  if (tileX < 0 || tileX >= level->width || tileY < 0 ||
      tileY >= level->height) {
    return false;
  }
  unsigned char tile = LevelGetTile(level, tileX, tileY);
  if (tile != 2) {
    return false;
  }
  float tileTop = (float)(tileY * TILE_SIZE);
  float originalBottom = originalY + boundingBox->height;
  if (originalBottom > tileTop + .01f) {
    return false;
  }
  boundingBox->y = tileTop - boundingBox->height;
  boundingBox->isOnGround = true;
  boundingBox->velocityX = 0.0f;
  return true;
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
  AxisAlignedBoundingBox box = {
      .x = player->entity.x,
      .y = player->entity.y,
      .width = player->entity.width,
      .height = player->entity.height,
      .velocityX = player->entity.velocityX,
      .velocityY = player->entity.velocityY,
      .isOnGround = false,
  };

  box.x += box.velocityX * deltaTime;
  CollisionResolveTileAxis(&box, level, true);
  box.y += box.velocityY * deltaTime;
  CollisionResolveTileAxis(&box, level, false);
  IsSnappedToPlaform(&box, originalY, level);
  player->entity.x = box.x;
  player->entity.y = box.y;
  player->entity.velocityX = box.velocityX;
  player->entity.velocityY = box.velocityY;
  player->isOnGround = box.isOnGround;
}

void PlayerUpdate(Player *player, const Input *input, const Level *level,
                  float deltaTime) {
  PlayerState previousPlayerState = player->currentPlayerState;

  /* ----- Cooldowns and Timers  ----------------------------------------- */
  DecreaseTimer(&player->dashCooldown, deltaTime);
  DecreaseTimer(&player->shootCooldown, deltaTime);
  DecreaseTimer(&player->jumpBufferTimer, deltaTime);
  DecreaseTimer(&player->shootAnimationTimer, deltaTime);
  if (player->isOnGround) {
    player->coyoteTimer = COYOTE_TIME;
  } else {
    DecreaseTimer(&player->coyoteTimer, deltaTime);
  }

  /* ----- Jump buffer and coyote timer --------------------------------- */
  if (input->isJumpJustPressed) {
    player->jumpBufferTimer = JUMP_BUFFER_TIME;
  }

  /* ----- Dash handling (state overrides normal movement) -------------- */
  if (input->isDashJustPressed && player->dashCooldown <= 0.0f &&
      !player->isDashing && player->isOnGround) {
    player->isDashing = true;
    player->dashTimer = DASH_DURATION;
    player->dashCooldown = DASH_COOLDOWN;
    if (player->isFacingRight) {
      player->entity.velocityX = DASH_SPEED;
    } else {
      player->entity.velocityX = -DASH_SPEED;
    }
    player->entity.velocityY = 0.0f;
    player->currentPlayerState = STATE_DASHING;
  }
  if (player->isDashing) {
    DecreaseTimer(&player->dashTimer, deltaTime);
    if (player->dashTimer <= 0.0f) {
      player->isDashing = false;
      player->entity.velocityX *= 0.3f;
    } else {
      player->entity.velocityY += GRAVITY * deltaTime * DASH_GRAVITY_SCALE;
      if (player->entity.velocityY > MAX_FALL_SPEED) {
        player->entity.velocityY = MAX_FALL_SPEED;
      }
      PlayerMoveAndResolve(player, level, deltaTime);
      player->currentPlayerState = STATE_DASHING;
      player->animationTimer += deltaTime;
      return;
    }
  }

  /* ----- Normal movement (only when not dashing) ----------------------- */
  float targetVelocityX = 0.0f;
  if (input->moveLeft) {
    targetVelocityX = -PLAYER_SPEED;
    if (!player->isWallSliding)
      player->isFacingRight = false;
  }
  if (input->moveRight) {
    targetVelocityX = PLAYER_SPEED;
    if (!player->isWallSliding)
      player->isFacingRight = true;
  }
  float acceleration;
  if (player->isOnGround) {
    acceleration = GROUND_ACCELERATION;
  } else {
    acceleration = AIR_ACCELERATION;
  }
  player->entity.velocityX +=
      (targetVelocityX - player->entity.velocityX) * acceleration * deltaTime;

  /* ----- Wall sliding detection ----------------------------------------- */
  player->isWallSliding = false;
  player->canWallJump = false;
  player->wallDirection = 0;
  if (!player->isOnGround && player->entity.velocityY > 0.0f) {
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

  /* ----- Jumping (normal) ------------------------------------------------ */
  if (player->jumpBufferTimer > 0.0f &&
      (player->isOnGround || player->coyoteTimer > 0.0f)) {
    player->entity.velocityY = -JUMP_FORCE;
    player->jumpBufferTimer = 0.0f;
    player->coyoteTimer = 0.0f;
    player->isOnGround = false;
    player->currentPlayerState = STATE_JUMPING;
  }

  /* ----- Wall jump --------------------------------------------------------*/
  if (player->jumpBufferTimer > 0.0f && player->canWallJump) {
    player->entity.velocityY = -WALL_JUMP_FORCE_Y;
    player->entity.velocityX = -player->wallDirection * WALL_JUMP_FORCE_X;
    player->isWallSliding = false;
    player->canWallJump = false;
    player->jumpBufferTimer = 0.0f;
    player->currentPlayerState = STATE_JUMPING;
    player->isFacingRight = (player->entity.velocityX > 0.0f);
  }

  /* ----- Variable jump height ---------------------------------------------*/
  if (!input->jumpDown && player->entity.velocityY < VARIABLE_JUMP_THRESHOLD &&
      !player->isWallSliding) {
    player->entity.velocityY *= VARIABLE_JUMP_MULTIPLIER;
  }

  /* ----- Gravity ----------------------------------------------------------*/
  if (!player->isWallSliding) {
    player->entity.velocityY += GRAVITY * deltaTime;
  } else {
    player->entity.velocityY += GRAVITY * deltaTime * WALL_SLIDE_GRAVITY_SCALE;
  }
  if (player->entity.velocityY > MAX_FALL_SPEED) {
    player->entity.velocityY = MAX_FALL_SPEED;
  }

  /* ----- Move and resolve collisions --------------------------------------*/
  PlayerMoveAndResolve(player, level, deltaTime);

  /* ----- Update animation state -------------------------------------------*/
  if (player->isDashing) {
    player->currentPlayerState = STATE_DASHING;
  } else if (player->isWallSliding) {
    player->currentPlayerState = STATE_WALL_SLIDING;
  } else if (!player->isOnGround) {
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

  // Reset animationTimer on state change
  if (player->currentPlayerState != previousPlayerState) {
    player->animationTimer = 0.0f;
  }
  player->animationTimer += deltaTime;
}
