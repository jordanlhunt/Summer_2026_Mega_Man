#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "collision.h"
#include "common.h"
#include "config.h"
#include "level.h"

#define MAX_PROJECTILES 16
#define PROJECTILE_WIDTH 11.0f
#define PROJECTILE_HEIGHT 11.0f
#define PROJECTILE_SPEED 420.0f
#define PROJECTILE_LIFETIME 1.2f
#define PROJECTILE_COOLDOWN 0.22f

typedef struct Projectile {
  float x;
  float y;
  float velocityX;
  float lifeTimer;
  bool isActive;

} Projectile;

/**
 * Finds a free slot in the projectile pool and activates a projectile traveling
 * from a point (orginX, originY) in the direction the player is facing. If
 * every slot is already active, the shit is sliently dropped
 */
void ProjectileSpawn(Projectile projectiles[MAX_PROJECTILES], float originX,
                     float originY, bool isFacingRight);

/**
 * Update all the projectiles by deltaTime, and freeing any that expire (collide
 * with solid enviorment or time expire)
 */
void ProjectileUpdateAll(Projectile projectiles[MAX_PROJECTILES],
                         const Level *level, float deltaTime);

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
                                    Player *player, const Input *input);

#endif
