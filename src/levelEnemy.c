#include "levelEnemy.h"
#include "config.h"
#include "graphics.h"
#include "level.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <wchar.h>

static void LevelEnemyRenderOneEnemy(const LevelEnemy *enemy,
                                     SDL_Renderer *renderer,
                                     const Camera *camera,
                                     SDL_Texture *enemyTexture) {
  int spriteWidth = LEVELENEMY_WIDTH;
  int spriteHeight = LEVELENEMY_FRAME_HEIGHT;
  SDL_FRect destinationRect = {.x = enemy->entity.x - camera->x -
                                    (spriteWidth - enemy->entity.width) / 2.0f,
                               .y = enemy->entity.y - camera->y -
                                    (spriteHeight - enemy->entity.height),
                               .w = (float)spriteWidth,
                               .h = (float)spriteHeight};
  if (enemyTexture == NULL) {
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &destinationRect);
    return;
  }

  int frameColumn;
  int frameRow = 0;
  switch (enemy->state) {
  case LEVELENEMY_STATE_GROUNDED: {
    frameColumn = ((int)(enemy->animationTimer * 4.0f) % 2);
    break;
  }
  case LEVELENEMY_STATE_FLYING: {
    frameColumn = 1 + ((int)(enemy->animationTimer * 4.0f) % 2);
    break;
  }
  case LEVELENEMY_STATE_TURNING: {
    frameColumn = 5;
    break;
  }
  default: {
    frameColumn = 0;
  }
  }
  SDL_FRect sourceRect = {.x = frameColumn * spriteWidth,
                          .y = frameRow * spriteHeight,
                          .w = spriteWidth,
                          .h = spriteHeight};
  SDL_FlipMode flip;
  if (enemy->isFacingRight) {
    flip = SDL_FLIP_NONE;
  } else {
    flip = SDL_FLIP_HORIZONTAL;
  }
  SDL_RenderTextureRotated(renderer, enemyTexture, &sourceRect,
                           &destinationRect, 0.0, NULL, flip);
}

void LevelEnemyInitialize(LevelEnemy *enemy, float x, float y,
                          LevelEnemyState initialState) {
  *enemy = (LevelEnemy){
      .entity =
          {
              .x = x,
              .y = y,
              .width = LEVELENEMY_WIDTH,
              .height = LEVELENEMY_HEIGHT,
              .velocityX = 0.0f,
              .velocityY = 0.0f,
              .isActive = true,
          },
      .hitPoints = 1,
      .animationTimer = 0.0f,
      .isFacingRight = true,
      .state = initialState,
  };
}

void LevelEnemyUpdate(LevelEnemy *enemy, float deltaTime) {
  if (!enemy->entity.isActive) {
    return;
  }

  switch (enemy->state) {
  case LEVELENEMY_STATE_GROUNDED:
    enemy->entity.velocityY = 0.0f;
    break;
  case LEVELENEMY_STATE_FLYING:
    enemy->entity.velocityY = sinf(enemy->animationTimer * 2.0f) * 8.0f;
    break;
  case LEVELENEMY_STATE_TURNING:
    enemy->entity.velocityX = 0.0f;
    enemy->entity.velocityY = 0.0f;
    break;
  }
  EntityMove(&enemy->entity, deltaTime);
  enemy->animationTimer += deltaTime;
}
void LevelEnemyUpdateAll(LevelEnemy enemies[], int enemyCount,
                         float deltaTime) {
  for (int i = 0; i < enemyCount; i++) {
    LevelEnemyUpdate(&enemies[i], deltaTime);
  }
}
void LevelEnemyRenderAll(const LevelEnemy enemies[], int count,
                         SDL_Renderer *renderer, const Camera *camera,
                         SDL_Texture *enemyTexture) {
  for (int i = 0; i < count; i++) {
    if (!enemies[i].entity.isActive) {
      continue;
    }
    LevelEnemyRenderOneEnemy(&enemies[i], renderer, camera, enemyTexture);
  }
}

void LevelEnemyApplyDamage(LevelEnemy *enemy, int damage) {
  if (!enemy->entity.isActive || damage == 0) {
    return;
  }
  enemy->hitPoints -= damage;
  if (enemy->hitPoints <= 0) {
    enemy->hitPoints = 0;
    enemy->entity.isActive = false;
  }
}
