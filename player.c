#include "player.h"
static void PlayerResolveCollision(Player *player, const Level *level,
                                   bool xAxis) {
  // Compute tile range based on current player position
  int startTileX = (int)floorf(player->x / TILE_SIZE);
  int startTileY = (int)floorf(player->y / TILE_SIZE);
  int endTileX = (int)floorf((player->x + player->width - 0.01f) / TILE_SIZE);
  int endTileY = (int)floorf((player->y + player->height - 0.01f) / TILE_SIZE);
  for (int tileY = startTileY; tileY <= endTileY; tileY++) {
    for (int tileX = startTileX; tileX <= endTileX; tileX++) {
      // Bounds check
      if (tileX < 0 || tileX >= level->width || tileY < 0 ||
          tileY >= level->height)
        continue;
      // Empty tile
      if (level->tiles[tileY * level->width + tileX] == 0)
        continue;
      float tileXPos = tileX * TILE_SIZE;
      float tileYPos = tileY * TILE_SIZE;
      // Resolve X axis
      if (xAxis) {
        if (player->velocityX > 0.0f) {
          player->x = tileXPos - player->width - 0.01f;
        } else if (player->velocityX < 0.0f) {
          player->x = tileXPos + TILE_SIZE + 0.01f;
        }
        player->velocityX = 0.0f;
      }
      // Resolve Y axis
      else {
        if (player->velocityY > 0.0f) { // landing on floor
          player->y = tileYPos - player->height - 0.01f;
          player->isOnGround = true;
        } else if (player->velocityY < 0.0f) { // hitting ceiling
          player->y = tileYPos + TILE_SIZE + 0.01f;
        }
        player->velocityY = 0.0f;
      }
      // Continue the loop – we may need to resolve other tiles
    }
  }
}
void PlayerUpdate(Player *player, const bool *keys, const Level *level,
                  float deltaTime) {
  // ----- Input --------------------------------------------------------
  bool dashKeyDown = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_Z];
  bool jumpKeyDown = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_J];
  bool moveLeftKeyDown = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A];
  bool moveRightKeyDown = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];
  bool dashKeyJustPressed = dashKeyDown && !player->dashKeyWasDown;
  bool jumpKeyJustPressed = jumpKeyDown && !player->jumpKeyWasDown;
  player->dashKeyWasDown = dashKeyDown;
  player->jumpKeyWasDown = jumpKeyDown;
  // ----- Jump buffer and coyote timer ---------------------------------
  if (jumpKeyJustPressed) {
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
  // ----- Dash handling (state overrides normal movement) -------------
  if (dashKeyJustPressed && player->dashCooldown <= 0.0f &&
      !player->isDashing && player->isOnGround) {
    // Start dash
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
    // Update dash timer
    player->dashTimer -= deltaTime;
    if (player->dashTimer <= 0.0f) {
      player->isDashing = false;
      player->velocityX *= 0.3f;
    }
    // Dash uses reduced gravity
    player->velocityY += GRAVITY * deltaTime * DASH_GRAVITY_SCALE;
    if (player->velocityY > MAX_FALL_SPEED)
      player->velocityY = MAX_FALL_SPEED;
    // Move and resolve collisions (only once)
    player->x += player->velocityX * deltaTime;
    PlayerResolveCollision(player, level, true);
    player->isOnGround = false; // reset for Y pass
    player->y += player->velocityY * deltaTime;
    PlayerResolveCollision(player, level, false);
    // Update animation timer and return – skip normal movement
    player->animationTimer += deltaTime;
    return;
  }
  // ----- Cooldown (only when not dashing) -----------------------------
  if (player->dashCooldown > 0.0f) {
    player->dashCooldown -= deltaTime;
    if (player->dashCooldown < 0.0f)
      player->dashCooldown = 0.0f;
  }
  // ----- Normal movement (only when not dashing) ---------------------
  float targetVelocityX = 0.0f;
  if (moveLeftKeyDown) {
    targetVelocityX = -PLAYER_SPEED;
    if (!player->isWallSliding)
      player->isFacingRight = false;
  }
  if (moveRightKeyDown) {
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
  // ----- Wall sliding detection --------------------------------------
  player->isWallSliding = false;
  player->canWallJump = false;
  player->wallDirection = 0;
  if (!player->isOnGround && player->velocityY > 0.0f) {
    int tileXLeft = (int)floorf((player->x - 1.0f) / TILE_SIZE);
    int tileXRight =
        (int)floorf((player->x + player->width + 1.0f) / TILE_SIZE);
    int tileYTop = (int)floorf((player->y + 2.0f) / TILE_SIZE);
    int tileYBottom =
        (int)floorf((player->y + player->height - 2.0f) / TILE_SIZE);
    bool isWallOnLeft = false;
    bool isWallOnRight = false;
    for (int tileY = tileYTop; tileY <= tileYBottom && tileY < level->height;
         tileY++) {
      if (tileY < 0)
        continue;
      if (tileXLeft >= 0 && tileXLeft < level->width &&
          level->tiles[tileY * level->width + tileXLeft] != 0)
        isWallOnLeft = true;
      if (tileXRight >= 0 && tileXRight < level->width &&
          level->tiles[tileY * level->width + tileXRight] != 0)
        isWallOnRight = true;
    }
    if (moveLeftKeyDown && isWallOnLeft) {
      player->isWallSliding = true;
      player->wallDirection = -1;
      player->isFacingRight = true;
    } else if (moveRightKeyDown && isWallOnRight) {
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
  // ----- Jumping (normal) --------------------------------------------
  if (player->jumpBufferTimer > 0.0f &&
      (player->isOnGround || player->coyoteTimer > 0.0f)) {
    player->velocityY = -JUMP_FORCE;
    player->jumpBufferTimer = 0.0f;
    player->coyoteTimer = 0.0f;
    player->isOnGround = false;
    player->currentPlayerState = STATE_JUMPING;
  }
  // ----- Wall jump ---------------------------------------------------
  if (player->jumpBufferTimer > 0.0f && player->canWallJump) {
    player->velocityY = -WALL_JUMP_FORCE_Y;
    player->velocityX = -player->wallDirection * WALL_JUMP_FORCE_X;
    player->isWallSliding = false;
    player->canWallJump = false;
    player->jumpBufferTimer = 0.0f;
    player->currentPlayerState = STATE_JUMPING;
    player->isFacingRight = (player->velocityX > 0.0f);
  }
  // ----- Variable jump height ----------------------------------------
  if (!jumpKeyDown && player->velocityY < VARIABLE_JUMP_THRESHOLD &&
      !player->isWallSliding) {
    player->velocityY *= VARIABLE_JUMP_MULTIPLIER;
  }
  // ----- Gravity -----------------------------------------------------
  if (!player->isWallSliding) {
    player->velocityY += GRAVITY * deltaTime;
  } else {
    player->velocityY += GRAVITY * deltaTime * WALL_SLIDE_GRAVITY_SCALE;
  }
  if (player->velocityY > MAX_FALL_SPEED)
    player->velocityY = MAX_FALL_SPEED;
  // ----- Move and resolve collisions (ONLY ONCE) ---------------------
  player->x += player->velocityX * deltaTime;
  PlayerResolveCollision(player, level, true);
  player->isOnGround = false; // reset for Y pass
  player->y += player->velocityY * deltaTime;
  PlayerResolveCollision(player, level, false);
  // ----- Update animation state --------------------------------------
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
void PlayerRender(const Player *player, SDL_Renderer *renderer, float cameraX,
                  float cameraY, SDL_Texture *sheet) {
  float drawX = player->x - cameraX - (SPRITE_WIDTH - player->width);
  float drawY = player->y - cameraY - (SPRITE_HEIGHT - player->height) * 2.0f;
  SDL_FRect destinationFRect = {drawX, drawY, SPRITE_WIDTH * 2.0f,
                                SPRITE_HEIGHT * 2.0f};
  if (sheet) {
    int frameCol = 0;
    int frameRow = player->styleRow;
    float animSpeed = 8.0f;
    switch (player->currentPlayerState) {
    case STATE_IDLE:
      frameCol = ANIMATION_IDLE_1;
      break;
    case STATE_RUNNING:
      animSpeed = 12.0f;
      frameCol = ((int)(player->animationTimer * animSpeed) % 6);
      frameRow = player->styleRow + 1;
      break;
    case STATE_JUMPING:
      frameCol = ANIMATION_JUMP;
      break;
    case STATE_FALLING:
      frameCol = ANIMATION_JUMP;
      break;
    case STATE_WALL_SLIDING:
      frameCol = ANIMATION_TURN;
      break;
    case STATE_DASHING:
    case STATE_SLIDING:
      frameCol = 6;
      frameRow = player->styleRow + 1;
      break;
    }
    SDL_FRect sourceFRect = {frameCol * SPRITE_WIDTH, frameRow * SPRITE_HEIGHT,
                             SPRITE_WIDTH, SPRITE_HEIGHT};
    SDL_FlipMode flip;
    if (player->isFacingRight) {
      flip = SDL_FLIP_NONE;
    } else {
      flip = SDL_FLIP_HORIZONTAL;
    }
    /* Dash Trail Effect */
    if (player->isDashing) {
      SDL_SetTextureColorMod(sheet, 255, 255, 255);
      SDL_FRect trailDestinationFRect = destinationFRect;
      if (player->isFacingRight) {
        trailDestinationFRect.x -= 10.0f;
      } else {
        trailDestinationFRect.x += 10.0f;
      }
      SDL_SetTextureAlphaMod(sheet, 128);
      SDL_RenderTextureRotated(renderer, sheet, &sourceFRect,
                               &trailDestinationFRect, 0.0, NULL, flip);
      SDL_SetTextureAlphaMod(sheet, 255);
    }
    SDL_RenderTextureRotated(renderer, sheet, &sourceFRect, &destinationFRect,
                             0.0, NULL, flip);
  } else {
    SDL_FRect renderRect = {drawX, drawY, SPRITE_WIDTH * 2.0f,
                            SPRITE_HEIGHT * 2.0f};
    switch (player->currentPlayerState) {
    case STATE_IDLE:
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      break;
    case STATE_RUNNING:
      SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
      break;
    case STATE_JUMPING:
      SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
      break;
    case STATE_FALLING:
      SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
      break;
    case STATE_WALL_SLIDING:
      SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      break;
    case STATE_DASHING:
      SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
      break;
    case STATE_SLIDING:
      SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
      break;
    }
    SDL_RenderFillRect(renderer, &renderRect);
  }
}
