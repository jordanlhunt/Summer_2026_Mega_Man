#include "graphics.h"
#include "game.h"
#include "player.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
void CameraUpdate(Camera *camera, float targetX, float targetY,
                  const Level *level, float deltaTime) {
  float maxCameraX = level->width * TILE_SIZE - SCREEN_WIDTH;
  float maxCameraY = level->height * TILE_SIZE - SCREEN_HEIGHT;
  float clampMaxX = maxCameraX;
  if (clampMaxX < 0) {
    clampMaxX = 0;
  }
  float clampMaxY = maxCameraY;
  if (clampMaxY < 0) {
    clampMaxY = 0;
  }
  targetX = SDL_clamp(targetX, 0, clampMaxX);
  targetY = SDL_clamp(targetY, 0, clampMaxY);
  camera->x += (targetX - camera->x) * CAMERA_SMOOTHING * deltaTime;
  camera->y += (targetY - camera->y) * CAMERA_SMOOTHING * deltaTime;
}
SDL_Texture *GraphicsLoadSpriteSheet(SDL_Renderer *renderer, const char *path) {
  SDL_Surface *spriteSheetSurface = IMG_Load(path);
  if (spriteSheetSurface == NULL) {
    SDL_Log("Unable to load sprite sheet: %s", SDL_GetError());
    return NULL;
  }
  SDL_Texture *texture =
      SDL_CreateTextureFromSurface(renderer, spriteSheetSurface);
  SDL_DestroySurface(spriteSheetSurface);
  if (texture == NULL) {
    SDL_Log("Unable to create texture from surface: %s", SDL_GetError());
    return NULL;
  }
  SDL_Log("Sprite sheet loaded successfully from: %s", path);
  return texture;
}
void GraphicsClear(SDL_Renderer *renderer) {
  // Cornflower Blue
  SDL_SetRenderDrawColor(renderer, 147, 204, 234, 255);
  SDL_RenderClear(renderer);
}
void GraphicsRenderLevel(const Level *level, SDL_Renderer *renderer,
                         const Camera *camera) {
  int startX = (int)floorf(camera->x / TILE_SIZE) - 1;
  int startY = (int)floorf(camera->y / TILE_SIZE) - 1;
  int endX = startX + (SCREEN_WIDTH / TILE_SIZE) + 3;
  int endY = startY + (SCREEN_HEIGHT / TILE_SIZE) + 3;
  for (int y = startY; y < endY; y++) {
    for (int x = startX; x < endX; x++) {
      if (x < 0 || y < 0) {
        continue;
      }
      if (x >= level->width || y >= level->height) {
        continue;
      }
      if (level->tiles[y * level->width + x] == 0) {
        continue;
      }
      SDL_FRect tile = {
          .x = x * TILE_SIZE - camera->x,
          .y = y * TILE_SIZE - camera->y,
          .w = TILE_SIZE,
          .h = TILE_SIZE,
      };
      bool isFloor = (y >= level->height - 3);
      bool isWall = (x == 0 || x == level->width - 1);
      if (y + 1 < level->height &&
          level->tiles[(y + 1) * level->width + x] == 0 && !isFloor) {
        SDL_SetRenderDrawColor(renderer, 255, 70, 60, 255);
      } else if (isWall) {
        SDL_SetRenderDrawColor(renderer, 100, 10, 10, 255);
      } else {
        SDL_SetRenderDrawColor(renderer, 200, 100, 0, 255);
      }
      SDL_RenderFillRect(renderer, &tile);
    }
  }
}
void GraphicsRenderPlayer(const Player *player, SDL_Renderer *renderer,
                          const Camera *camera, SDL_Texture *sheet) {
  float drawX =
      player->x - camera->x -
      (SPRITE_WIDTH * 2.0f - player->width) / 2.0f; // center horizontally
  float drawY =
      player->y - camera->y -
      (SPRITE_HEIGHT * 2.0f - player->height); // align feet to hitbox bottom
  SDL_FRect destinationFRect = {drawX, drawY, SPRITE_WIDTH * 2.0f,
                                SPRITE_HEIGHT * 2.0f};
  if (sheet) {
    int frameColumn = 0;
    int frameRow = player->styleRow;
    float animationSpeed = 8.0f;
    switch (player->currentPlayerState) {
    case STATE_IDLE:
      frameColumn = ANIMATION_IDLE_1;
      break;
    case STATE_RUNNING:
      animationSpeed = 12.0f;
      frameColumn = ((int)(player->animationTimer * animationSpeed) % 6);
      frameRow = player->styleRow + 1;
      break;
    case STATE_JUMPING:
      frameColumn = ANIMATION_JUMP;
      break;
    case STATE_FALLING:
      frameColumn = ANIMATION_JUMP;
      break;
    case STATE_WALL_SLIDING:
      frameColumn = ANIMATION_TURN;
      break;
    case STATE_DASHING:
    case STATE_SLIDING:
      frameColumn = 6;
      frameRow = player->styleRow + 1;
      break;
    }
    /**
     * Shooting temporarily overrides the movement animation
     * shooting frames are on the first row of each style blook
     */
    if (player->shootAnimationTimer > 0.0f) {
      frameRow = player->styleRow;
      float elpasedShootTime =
          SHOOT_ANIMATION_DURATION - player->shootAnimationTimer;
      if (elpasedShootTime < SHOOT_FLASH_DURATION) {
        frameColumn = ANIMATION_SHOOT_FLASH;
      } else {
        frameColumn = ANIMATION_SHOOT_END;
      }
    }

    SDL_FRect sourceFRect = {frameColumn * SPRITE_WIDTH,
                             frameRow * SPRITE_HEIGHT, SPRITE_WIDTH,
                             SPRITE_HEIGHT};
    SDL_FlipMode flip;
    if (player->isFacingRight) {
      flip = SDL_FLIP_NONE;
    } else {
      flip = SDL_FLIP_HORIZONTAL;
    }

    /* Dash trail effect */
    if (player->isDashing) {
      // Number of ghosts and their spacing
      const int numberOfGhostTrails = 3; // how many ghosts
      const float spacing = 1.5f;        // multiplier for offset distance
      const Uint8 alphas[] = {200, 150, 90, 50}; // alpha per ghost

      for (int i = 0; i < numberOfGhostTrails; i++) {
        // Offset increases with each ghost
        float offset = DASH_TRAIL_OFFSET * (i + 1) * spacing;
        SDL_FRect trailRect = destinationFRect;
        if (player->isFacingRight) {
          trailRect.x -= offset; // trail behind (opposite to facing)
        } else {
          trailRect.x += offset;
        }

        SDL_SetTextureAlphaMod(sheet, alphas[i]);
        SDL_RenderTextureRotated(renderer, sheet, &sourceFRect, &trailRect, 0.0,
                                 NULL, flip);
      }
      // Restore alpha for the main player sprite
      SDL_SetTextureAlphaMod(sheet, 255);
    }
    SDL_RenderTextureRotated(renderer, sheet, &sourceFRect, &destinationFRect,
                             0.0, NULL, flip);
  } else {
    /* Fallback so the game is still visible/debuggable without art. */
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
void GraphicsPresent(SDL_Renderer *renderer) { SDL_RenderPresent(renderer); }
