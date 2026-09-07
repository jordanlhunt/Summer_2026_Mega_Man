#ifndef LEVELENEMY_H
#define LEVELENEMY_H

#include "common.h"
#include "entity.h"
#include "forwarddeclares.h"

typedef enum LevelEnemyState {
  LEVELENEMY_STATE_GROUNDED,
  LEVELENEMY_STATE_FLYING,
  LEVELENEMY_STATE_TURNING
} LevelEnemyState;

typedef struct LevelEnemy {
  Entity entity;

  int hitPoints;
  float animationTimer;
  bool isFacingRight;
  LevelEnemyState state;
} LevelEnemy;

/**
 * Initializes an enemy at the supplied world position.
 */
void LevelEnemyInitialize(LevelEnemy *enemy, float x, float y,
                          LevelEnemyState initialState);

/**
 * Applies damage and deactivates the enemy when its health reaches zero.
 */
void LevelEnemyApplyDamage(LevelEnemy *enemy, int damage);

/**
 * Updates one active enemy.
 */
void LevelEnemyUpdate(LevelEnemy *enemy, float deltaTime);

/**
 * Updates all enemies in the supplied array.
 */
void LevelEnemyUpdateAll(LevelEnemy enemies[], int enemyCount, float deltaTime);

/**
 * Renders all active enemies in the supplied array.
 */
void LevelEnemyRenderAll(const LevelEnemy enemies[], int enemyCount,
                         SDL_Renderer *renderer, const Camera *camera,
                         SDL_Texture *enemyTexture);

#endif
