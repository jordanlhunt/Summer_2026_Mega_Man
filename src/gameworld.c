#include "gameworld.h"
#include "config.h"
#include "graphics.h"
#include "level.h"
#include "levelEnemy.h"
#include "player.h"
#include "projectile.h"

/**
 * Helper functions
 */
static bool DoorOverlapsPlayer(const DoorTransition *doorTransition,
                               const Player *player) {
  const float padding = 6.0f;
  float doorX = (float)(doorTransition->tileX * TILE_SIZE);
  float doorY = (float)(doorTransition->tileY * TILE_SIZE);
  float playerX = player->entity.x - padding;
  float playerY = player->entity.y - padding;
  float playerWidth = player->entity.width + padding * 2.0f;
  float playerHeight = player->entity.height + padding * 2.0f;
  return (playerX < doorX + TILE_SIZE) && (playerX + playerWidth > doorX) &&
         (playerY < doorY + TILE_SIZE) && (playerY + playerHeight > doorY);
}

static void PlacePlayerAtDoorSpawn(Player *player, int spawnTileX,
                                   int spawnTileY) {
  player->entity.x =
      spawnTileX * TILE_SIZE + (TILE_SIZE - player->entity.width) * .5f;
  player->entity.y = spawnTileY * TILE_SIZE + TILE_SIZE - player->entity.height;
}
static void PlayerResetForRoomTransition(Player *player) {
  player->entity.velocityX = 0.0f;
  player->entity.velocityY = 0.0f;
  player->entity.isOnGround = false;
  player->isDashing = false;
  player->isWallSliding = false;
  player->dashTimer = 0.0f;
  player->wallSlideTimer = 0.0f;
  player->coyoteTimer = 0.0f;
  player->jumpBufferTimer = 0.0f;
  player->shootAnimationTimer = 0.0f;
  player->animationTimer = 0.0f;
  player->currentPlayerState = STATE_IDLE;
}
static void CameraSnapToPlayer(Camera *camera, const Player *player,
                               const Level *level) {}

/**
 * End of Helper functions
 */

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
                     SDL_Texture *projectileTexture,
                     SDL_Texture *levelEnemyTexture) {
  GraphicsRenderLevel(&gameWorld->level, renderer, &gameWorld->camera);
  GraphicsRenderPlayer(&gameWorld->player, renderer, &gameWorld->camera);
  ProjectileRenderAll(gameWorld->projectiles, renderer, gameWorld->camera.x,
                      gameWorld->camera.y, projectileTexture);
  LevelEnemyRenderAll(gameWorld->level.levelEnemies,
                      LevelGetEnemyCount(&gameWorld->level), renderer,
                      &gameWorld->camera, levelEnemyTexture);
}