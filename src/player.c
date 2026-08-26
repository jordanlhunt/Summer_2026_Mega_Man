#include "player.h"

static void DecreaseTimer(float *timer, float deltaTime) {
  if (*timer > 0.0f) {
    *timer -= deltaTime;
    if (*timer < 0.0f) {
      *timer = 0.0f;
    }
  }
}

/* Integrates velocity into position and resolves tile collisions on both
 * axes, matching the original move-x/resolve-x, move-y/resolve-y order.
 * Both the dash path and the normal movement path in PlayerUpdate used to
 * duplicate this block; pulling it out here is the main DRY win of moving
 * collision code into its own module. */
static void PlayerMoveAndResolve(Player *player, const Level *level,
                                 float deltaTime) {
  AxisAlignedBoundingBox box = {
      .x = player->x,
      .y = player->y,
      .width = player->width,
      .height = player->height,
      .velocityX = player->velocityX,
      .velocityY = player->velocityY,
      .isOnGround = false,
  };

  box.x += box.velocityX * deltaTime;
  CollisionResolveTileAxis(&box, level, true);

  box.y += box.velocityY * deltaTime;
  CollisionResolveTileAxis(&box, level, false);

  player->x = box.x;
  player->y = box.y;
  player->velocityX = box.velocityX;
  player->velocityY = box.velocityY;
  player->isOnGround = box.isOnGround;
}
void PlayerUpdate(Player *player, const Input *input, const Level *level,
                  float deltaTime) {
  DecreaseTimer(&player->dashCooldown, deltaTime);
  DecreaseTimer(&player->shootCooldown, deltaTime);
  /* ----- Jump buffer and coyote timer --------------------------------- */
  if (input->isJumpJustPressed) {
    player->jumpBufferTimer = JUMP_BUFFER_TIME;
  }
  if (player->jumpBufferTimer > 0.0f) {
    player->jumpBufferTimer -= deltaTime;
    if (player->jumpBufferTimer < 0.0f)
      player->jumpBufferTimer = 0.0f;
  }
  if (player->isOnGround) {
    player->coyoteTimer = COYOTE_TIME;
  } else if (player->coyoteTimer > 0.0f) {
    player->coyoteTimer -= deltaTime;
    if (player->coyoteTimer < 0.0f)
      player->coyoteTimer = 0.0f;
  }

  /* ----- Dash handling (state overrides normal movement) -------------- */
  if (input->isDashJustPressed && player->dashCooldown <= 0.0f &&
      !player->isDashing && player->isOnGround) {
    player->isDashing = true;
    player->dashTimer = DASH_DURATION;
    player->dashCooldown = DASH_COOLDOWN;
    if (player->isFacingRight) {
      player->velocityX = DASH_SPEED;
    } else {
      player->velocityX = -DASH_SPEED;
    }
    player->velocityY = 0.0f;
    player->currentPlayerState = STATE_DASHING;
  }

  if (player->isDashing) {
    player->dashTimer -= deltaTime;

    if (player->dashTimer <= 0.0f) {
      player->dashTimer = 0.0f;
      player->isDashing = false;
      player->velocityX *= 0.3f;
    } else {
      player->velocityY += GRAVITY * deltaTime * DASH_GRAVITY_SCALE;

      if (player->velocityY > MAX_FALL_SPEED) {
        player->velocityY = MAX_FALL_SPEED;
      }

      PlayerMoveAndResolve(player, level, deltaTime);
      player->currentPlayerState = STATE_DASHING;
      player->animationTimer += deltaTime;
      return;
    }
  }

  /* ----- Cooldown ------------------------------------------------------ */
  if (player->dashCooldown > 0.0f) {
    player->dashCooldown -= deltaTime;
    if (player->dashCooldown < 0.0f)
      player->dashCooldown = 0.0f;
  }
  if (player->shootCooldown > 0.0f) {
    player->shootCooldown -= deltaTime;
    if (player->shootCooldown < 0.0f) {
      player->shootCooldown = 0.0f;
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
  player->velocityX +=
      (targetVelocityX - player->velocityX) * acceleration * deltaTime;

  /* ----- Wall sliding detection ----------------------------------------- */
  player->isWallSliding = false;
  player->canWallJump = false;
  player->wallDirection = 0;

  if (!player->isOnGround && player->velocityY > 0.0f) {
    bool isWallOnLeft =
        CollisionCheckWallLeft(level, player->x, player->y, player->height);
    bool isWallOnRight = CollisionCheckWallRight(level, player->x, player->y,
                                                 player->width, player->height);

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
    player->velocityY = fminf(player->velocityY, WALL_SLIDE_SPEED);
    player->wallSlideTimer += deltaTime;
    player->canWallJump = true;
  } else {
    player->wallSlideTimer = 0.0f;
  }

  /* ----- Jumping (normal) ------------------------------------------------ */
  if (player->jumpBufferTimer > 0.0f &&
      (player->isOnGround || player->coyoteTimer > 0.0f)) {
    player->velocityY = -JUMP_FORCE;
    player->jumpBufferTimer = 0.0f;
    player->coyoteTimer = 0.0f;
    player->isOnGround = false;
    player->currentPlayerState = STATE_JUMPING;
  }

  /* ----- Wall jump --------------------------------------------------------*/
  if (player->jumpBufferTimer > 0.0f && player->canWallJump) {
    player->velocityY = -WALL_JUMP_FORCE_Y;
    player->velocityX = -player->wallDirection * WALL_JUMP_FORCE_X;
    player->isWallSliding = false;
    player->canWallJump = false;
    player->jumpBufferTimer = 0.0f;
    player->currentPlayerState = STATE_JUMPING;
    player->isFacingRight = (player->velocityX > 0.0f);
  }

  /* ----- Variable jump height ---------------------------------------------*/
  if (!input->jumpDown && player->velocityY < VARIABLE_JUMP_THRESHOLD &&
      !player->isWallSliding) {
    player->velocityY *= VARIABLE_JUMP_MULTIPLIER;
  }

  /* ----- Gravity ----------------------------------------------------------*/
  if (!player->isWallSliding) {
    player->velocityY += GRAVITY * deltaTime;
  } else {
    player->velocityY += GRAVITY * deltaTime * WALL_SLIDE_GRAVITY_SCALE;
  }
  if (player->velocityY > MAX_FALL_SPEED)
    player->velocityY = MAX_FALL_SPEED;

  /* ----- Move and resolve collisions --------------------------------------*/
  PlayerMoveAndResolve(player, level, deltaTime);

  /* ----- Update animation state -------------------------------------------*/
  if (player->isDashing) {
    player->currentPlayerState = STATE_DASHING;
  } else if (player->isWallSliding) {
    player->currentPlayerState = STATE_WALL_SLIDING;
  } else if (!player->isOnGround) {
    if (player->velocityY < 0) {
      player->currentPlayerState = STATE_JUMPING;
    } else {
      player->currentPlayerState = STATE_FALLING;
    }
  } else if (fabsf(player->velocityX) > 10.0f) {
    player->currentPlayerState = STATE_RUNNING;
  } else {
    player->currentPlayerState = STATE_IDLE;
  }

  player->animationTimer += deltaTime;
}
