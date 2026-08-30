#include "levelEnemy.h"

void LevelEnemyUpdate(LevelEnemy *levelEnemy, const Level *level,
                      float deltaTime) {
  if (levelEnemy->isActive) {
    return;
  }
  /**
   * TODO: Added Animations and stuff
   */
}

void LevelEnemyUpdateAll(LevelEnemy *levelEnemies, int levelEnemyCount,
                         const Level *level, float deltaTime) {
  for (int i = 0; i < levelEnemyCount; i++) {
    LevelEnemyUpdate(&levelEnemies[i], level, deltaTime);
  }
}
void LevelEnemyRenderAll(LevelEnemy *levelEnemies, SDL_Renderer *renderer,
                         int levelEnemyCount, SDL_Texture *levelEnemyTexture) {
  for (int i = 0; i < levelEnemyCount; i++) {
    if (!levelEnemies[i].isActive) {
      continue;
    }
    SDL_FRect
  }
}
