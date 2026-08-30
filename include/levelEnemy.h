#ifndef LEVELENEMY_H
#define LEVELENEMY_H

#include "common.h"
#include "config.h"

typedef struct Level Level;
typedef struct Camera Camera;

/**
 * LevelEnemy State Machine for animation control
 */
typedef enum LevelEnemyState {
  LEVELENEMY_STATE_GROUNDED,
  LEVELENEMY_STATE_FLYING,
  LEVELENEMY_STATE_TURNING
} LevelEnemyState;

typedef struct LevelEnemy {
  float x;
  float y;
  float velocityX;
  float velocityY;
  float width;
  float height;
  int hitPoints;
  float animationTimer;
  bool isActive;
  bool isFacingRight;
  LevelEnemyState levelEnemyState;
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
                         SDL_Texture *enemyTexture);
#endif
