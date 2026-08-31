#include "levelEnemy.h"
#include "config.h"
#include "graphics.h"
#include "level.h"
#include <assert.h>
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

void LevelEnemyUpdate(LevelEnemy *enemy, const Level *level, float deltaTime) {
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
                         const Level *level, float deltaTime) {
  assert(deltaTime >= 0.0f);
  for (int i = 0; i < enemyCount; i++) {
    LevelEnemyUpdate(&enemies[i], level, deltaTime);
  }
}
void LevelEnemyRenderAll(const LevelEnemy enemies[], int count,
                         SDL_Renderer *renderer, const Camera *camera,
                         SDL_Texture *enemyTexture) {
  for (int i = 0; i < count; i++) {
    if (!enemies[i].entity.isActive)
      continue;
    SDL_FRect destinationRect = {.x = enemies[i].entity.x - camera->x,
                                 .y = enemies[i].entity.y - camera->y,
                                 .w = enemies[i].entity.width,
                                 .h = enemies[i].entity.height};
    if (enemyTexture == NULL) {
      SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      SDL_RenderFillRect(renderer, &destinationRect);
      continue;
    }
    int spriteW = LEVELENEMY_WIDTH;
    int spriteH = LEVELENEMY_FRAME_HEIGHT;
    int frameColumn;
    int frameRow = 0;
    switch (enemies[i].state) {
    case LEVELENEMY_STATE_GROUNDED:
      frameColumn = ((int)(enemies[i].animationTimer * 4.0f) % 2);
      break;
    case LEVELENEMY_STATE_FLYING:
      frameColumn = 2 + ((int)(enemies[i].animationTimer * 6.0f) % 4);
      break;
    case LEVELENEMY_STATE_TURNING:
      frameColumn = 5;
      break;
    default:
      frameColumn = 0;
    }
    SDL_FRect sourceRect = {.x = frameColumn * spriteW,
                            .y = frameRow * spriteH,
                            .w = spriteW,
                            .h = spriteH};
    destinationRect.x -= (spriteW - enemies[i].entity.width) / 2.0f;
    destinationRect.y -= (spriteH - enemies[i].entity.height);
    SDL_FlipMode flip;
    if (enemies[i].isFacingRight) {
      flip = SDL_FLIP_NONE;
    } else {
      flip = SDL_FLIP_HORIZONTAL;
    }
    SDL_RenderTextureRotated(renderer, enemyTexture, &sourceRect,
                             &destinationRect, 0.0, NULL, flip);
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