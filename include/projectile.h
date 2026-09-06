#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "collision.h"
#include "common.h"
#include "config.h"
#include "entity.h"
#include "forwarddeclares.h"

#include "level.h"
#include "levelEnemy.h"
#include "player.h"

#define MAX_PROJECTILES 16
#define PROJECTILE_WIDTH 11.0f
#define PROJECTILE_HEIGHT 11.0f
#define PROJECTILE_SPEED 420.0f
#define PROJECTILE_LIFETIME 1.2f
#define PROJECTILE_COOLDOWN 0.22f
#define PROJECTILE_SUBSTEP_SIZE (TILE_SIZE * 0.5f)
#define PROJECTILE_MUZZLE_OFFSET_Y 7.0f
typedef struct Projectile {
  float lifeTimer;
  Entity entity;
} Projectile;
/**
 * Finds a free slot in the projectile pool and activates a projectile traveling
 * from a point (originX, originY) in the direction the player is facing. If
 * every slot is already active, the shot is sliently dropped
 */
bool ProjectileSpawn(Projectile projectiles[MAX_PROJECTILES], float originX,
                     float originY, bool isFacingRight);
/**
 * Update all active projectiles: move, check lifetime and collision.
 */
void ProjectileUpdateAll(Projectile projectiles[MAX_PROJECTILES], Level *level,
                         float deltaTime);
/**
 * Draws all the active projectile with the shotTexture, offset by the camera
 * position
 */
void ProjectileRenderAll(const Projectile projectiles[MAX_PROJECTILES],
                         SDL_Renderer *renderer, float cameraX, float cameraY,
                         SDL_Texture *projectileTexture);
/**
 * Handles the player's shooting input.
 * Updates the player's shootCooldown, and spawns a projectile if conditions are
 * met.
 */
void ProjectileHandlePlayerShooting(Projectile projectiles[MAX_PROJECTILES],
                                    Player *player, bool isShootJustPressed);
/**
 * Handles projectile collision from the player to a levelEnemy
 * TODO: Make this a general soultion for all projectiles
 */
void HandleProjectileEntityCollision(Projectile projectiles[MAX_PROJECTILES],
                                     LevelEnemy levelEnemies[], int enemyCount);
#endif
