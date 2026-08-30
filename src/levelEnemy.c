#include "levelEnemy.h"
#include "collision.h"
#include "config.h"
#include "graphics.h"

void LevelEnemyUpdate(LevelEnemy *levelEnemy, const Level *level,
                      float deltaTime) {
  if (!levelEnemy->isActive) {
    return;
  }
  /**
   * TODO: Added Animations and stuff
   */
  // Advance animation timer (drives frame selection in render)
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
    if (!enemies[i].isActive)
      continue;

    SDL_FRect destinationRect = {.x = enemies[i].x - camera->x,
                                 .y = enemies[i].y - camera->y,
                                 .w = enemies[i].width,
                                 .h = enemies[i].height};

    if (enemyTexture == NULL) {
      SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      SDL_RenderFillRect(renderer, &destinationRect);
      continue;
    }

    int spriteW = LEVELENEMY_WIDTH;
    int spriteH = LEVELENEMY_HEIGHT;
    int framesPerRow = LEVELENEMY_COLUMNS;
    int frameColumn = frameColumn =
        ((int)(enemies[i].animationTimer * 4.0f) % 2);
    int frameRow = 0;

    SDL_FRect sourceRect = {
        .x = frameColumn * spriteW, .y = 0, .w = spriteW, .h = spriteH};

    destinationRect.x -= (spriteW - enemies[i].width) / 2.0f;
    destinationRect.y -= (spriteH - enemies[i].height); // Aligns feet to bottom

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
