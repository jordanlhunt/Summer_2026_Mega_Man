#ifndef LEVELENEMY_H
#define LEVELENEMY_H
#include "collision.h"
#include "common.h"
#include "config.h"
#include "graphics.h"
#include "level.h"

typedef struct LevelEnemy {
  float x;
  float y;
  float velocityX;
  float width;
  float height;
  int hitPoints;
  float animationTimer;
  bool isActive;
  bool isFacingRight;
} LevelEnemy;
/**
 * Updates the LevelEnemy
 */
void LevelEnemyUpdate(LevelEnemy *levelEnemy, const Level *level,
                      float deltaTime);
/**
 * Loops through all the LevelEnemies and updates them, will expand this in the
 * future to update them based on their classification
 */
void LevelEnemyUpdateAll(LevelEnemy levelEnemies[], int levelEnemyCount,
                         const Level *level, float deltaTime);
/**
 * Loops through all the LevelEnemies and draw them, will expand this in the
 * future to draw them based on their classification
 */
void LevelEnemyRenderAll(const LevelEnemy enemies[], int count,
                         SDL_Renderer *renderer, const Camera *camera,
                         SDL_Texture *enemyTexture)
#endif
