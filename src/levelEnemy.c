#include "levelEnemy.h"
#include "collision.h"
#include "config.h"
#include "graphics.h"

void LevelEnemyUpdate(LevelEnemy *levelEnemy, const Level *level,
                      float deltaTime) {
  if (!levelEnemy->entity.isActive) {
    return;
  }

  switch (levelEnemy->enemyState) {
  case ENEMY_STATE_GROUNDED:
    levelEnemy->entity.velocityY = 0.0f;
    break;

  case ENEMY_STATE_FLYING:
    levelEnemy->entity.velocityY =
        sinf(levelEnemy->animationTimer * 2.0f) * 0.5f;
    break;

  case ENEMY_STATE_TURNING:
    break;
  }

  levelEnemy->entity.x += levelEnemy->entity.velocityX * deltaTime;
  levelEnemy->entity.y += levelEnemy->entity.velocityY * deltaTime;
  levelEnemy->animationTimer += deltaTime;
}

void LevelEnemyUpdateAll(LevelEnemy *levelEnemies, int levelEnemyCount,
                         const Level *level, float deltaTime) {
  for (int i = 0; i < levelEnemyCount; i++) {
    LevelEnemyUpdate(&levelEnemies[i], level, deltaTime);
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
    int framesPerRow = LEVELENEMY_COLUMNS;

    int frameColumn;
    int frameRow = 0;

    switch (enemies[i].enemyState) {
    case ENEMY_STATE_GROUNDED:
      frameColumn = ((int)(enemies[i].animationTimer * 4.0f) % 2);
      break;

    case ENEMY_STATE_FLYING:
      frameColumn = 2 + ((int)(enemies[i].animationTimer * 6.0f) % 4);
      break;

    case ENEMY_STATE_TURNING:
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
