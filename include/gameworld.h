#ifndef GAMEWORLD_H
#define GAMEWORLD_H
#include "collision.h"
#include "common.h"
#include "config.h"
#include "graphics.h"
#include "input.h"
#include "level.h"
#include "levelEnemy.h"
#include "player.h"
#include "projectile.h"

typedef struct GameWorld {
  Player player;
  Level level;
  Camera camera;
  Projectile projectiles[MAX_PROJECTILES];
} GameWorld;

/**
 * Loads 'levelPath' into the world, resets the camera, places the player at the
 * level's spawn point, and clears any existing projectiles
 */
bool GameWorldLoadLevel(GameWorld *gameWorld, const char *levelPath);
void GameWorldShutdown(GameWorld *gameWorld);

/**
 * Runs one fixed-timestep update of everything in the world: player,
 * projectiles, enemies, collision, and camera
 */
void GameWorldUpdate(GameWorld *gameWorld, const Input *input, float deltaTime);

/**
 * Renders the level, player, projectiles, and enemies in that order
 */
void GameWorldRender(const GameWorld *gameWorld, SDL_Renderer *renderer,
                     SDL_Texture *projectileTexture,
                     SDL_Texture *levelEnemyTexture);
#endif