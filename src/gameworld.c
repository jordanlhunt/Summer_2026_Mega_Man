#include "gameworld.h"
#include "graphics.h"
#include "level.h"
#include "levelEnemy.h"
#include "projectile.h"

bool GameWorldLoadLevel(GameWorld *gameWorld, const char *levelPath) {
  if (!LevelLoadFromFile(&gameWorld->level, levelPath)) {
    return false;
  }
  // Reset the camera
  gameWorld->camera.x = 0.0f;
  gameWorld->camera.y = 0.0f;
  // Spawn the player
  gameWorld->player.entity.x = gameWorld->level.playerSpawnX;
  gameWorld->player.entity.y = gameWorld->level.playerSpawnY;
  // Initialize the player projectile pool
  memset(gameWorld->projectiles, 0, sizeof(gameWorld->projectiles));
  return true;
}

void GameWorldShutdown(GameWorld *gameWorld) { LevelFree(&gameWorld->level); }
void GameWorldUpdate(GameWorld *gameWorld, const Input *input,
                     float deltaTime) {
  PlayerUpdate(&gameWorld->player, input, &gameWorld->level, deltaTime);
  ProjectileHandlePlayerShooting(gameWorld->projectiles, &gameWorld->player,
                                 input->isShootJustPressed);
  ProjectileUpdateAll(gameWorld->projectiles, &gameWorld->level, deltaTime);
  LevelEnemyUpdateAll(gameWorld->level.levelEnemies,
                      LevelGetEnemyCount(&gameWorld->level), deltaTime);
  HandleProjectileEntityCollision(gameWorld->projectiles,
                                  gameWorld->level.levelEnemies,
                                  LevelGetEnemyCount(&gameWorld->level));
  float targetCameraX = gameWorld->player.entity.x - SCREEN_WIDTH / 2.0f;
  float targetCameraY = gameWorld->player.entity.y - SCREEN_HEIGHT / 2.0f;
  CameraUpdate(&gameWorld->camera, targetCameraX, targetCameraY,
               &gameWorld->level, deltaTime);
}
void GameWorldRender(const GameWorld *gameWorld, SDL_Renderer *renderer,
                     SDL_Texture *playerTexture, SDL_Texture *projectileTexture,
                     SDL_Texture *levelEnemyTexture) {
  GraphicsRenderLevel(&gameWorld->level, renderer, &gameWorld->camera);
  GraphicsRenderPlayer(&gameWorld->player, renderer, &gameWorld->camera,
                       playerTexture);
  ProjectileRenderAll(gameWorld->projectiles, renderer, gameWorld->camera.x,
                      gameWorld->camera.y, projectileTexture);
  LevelEnemyRenderAll(gameWorld->level.levelEnemies,
                      LevelGetEnemyCount(&gameWorld->level), renderer,
                      &gameWorld->camera, levelEnemyTexture);
}