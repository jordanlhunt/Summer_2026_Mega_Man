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
  char currentLevelPath[MAX_LEVEL_PATH_LENGTH];
} GameWorld;

typedef enum Direction { LEFT, RIGHT, UP, DOWN } Direction;

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
                     SDL_Texture *levelEnemyTexture, SDL_Texture *doorTexture);
bool GameWorldChangeLevelAtDoor(GameWorld *gameWorld,
                                const char *targetLevelPath, int targetDoorX,
                                int targetDoorY, int spawnOffsetX,
                                int spawnOffSetY);
/**
 * If "use" was just pressed and the player is standing in a door, transitions
 * the world to that door's target room. Returns true if a transition
 * happened this frame. The caller should skip the rest of its update, since
 * the world just changed underneath it.
 */
bool GameWorldHandleDoorUse(GameWorld *gameWorld, const Input *input);
/**
 * If the player walks of the edge of a map
 */
bool GameWorldChangeLevelAtEdge(GameWorld *gameWorld, const char *path,
                                Direction exitDirection);
#endif