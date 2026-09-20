#include "gameworld.h"
#include "config.h"
#include "graphics.h"
#include "level.h"
#include "levelEnemy.h"
#include "player.h"
#include "projectile.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <string.h>

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
                               const Level *level) {
  float maxX = LevelGetWidthPixels(level) - SCREEN_WIDTH;
  float maxY = LevelGetHeightPixels(level) - SCREEN_HEIGHT;
  if (maxX < 0.0f) {
    maxX = 0.0f;
  }
  if (maxY < 0.0f) {
    maxY = 0.0f;
  }
  camera->x = SDL_clamp(player->entity.x - SCREEN_WIDTH / 2.0f, 0.0f, maxX);
  camera->y = SDL_clamp(player->entity.y - SCREEN_HEIGHT / 2.0f, 0.0f, maxY);
}

static void ResolveLevelPath(char *out, size_t outSize,
                             const char *currentLevelPath,
                             const char *targetFileName) {
  const char *lastSlash = strchr(currentLevelPath, '/');
  if (lastSlash == NULL) {
    snprintf(out, outSize, "%s", targetFileName);
    return;
  }
  int directoryLength = (int)(lastSlash - currentLevelPath + 1);
  snprintf(out, outSize, "%.*s%s", directoryLength, currentLevelPath,
           targetFileName);
}

// Extracted commit from the GameWOrldChangeLevelAtDoor()
static void GameWorldCommitTransition(GameWorld *gameWorld, Level *newLevel,
                                      const *pathToNewLevel) {
  LevelFree(&gameWorld->level);
  gameWorld->level = *newLevel;
  PlayerResetForRoomTransition(&gameWorld->player);
  memset(gameWorld->projectiles, 0, sizeof(gameWorld->projectiles));
  strncpy(gameWorld->currentLevelPath, pathToNewLevel,
          MAX_LEVEL_PATH_LENGTH - 1);
  gameWorld->currentLevelPath[MAX_LEVEL_PATH_LENGTH - 1] = '\0';
}

// The Mega-Man transition from the edge of a room into the next
static void PlacePlayerForEdgeArrival(Player *player, const Level *level,
                                      Direction exitDirection) {
  float roomWidthInPixels = LevelGetWidthPixels(level);
  float roomHeightInPixels = LevelGetHeight(level);
  const float inset = 2.0f;
  switch (exitDirection) {
  case RIGHT: {
    player->entity.x = inset;

    break;
  }
  case LEFT: {
    player->entity.x = roomWidthInPixels - player->entity.width - inset;
    break;
  }
  case DOWN: {
    player->entity.y = inset;
    break;
  }
  case UP: {
    player->entity.y = roomHeightInPixels - player->entity.height - inset;
    break;
  }
  default:
    break;
  }
  player->entity.x =
      SDL_clamp(player->entity.x, 0, roomWidthInPixels - player->entity.width);
  player->entity.y = SDL_clamp(player->entity.y, 0,
                               roomHeightInPixels - player->entity.height);
}

static const char *EdgeTargetFor(const Level *level, Direction direction) {
  switch (direction) {
  case LEFT: {
    return level->edgeLeft;
  }
  case RIGHT: {
    return level->edgeRight;
  }
  case UP: {
    return level->edgeUp;
  }
  case DOWN: {
    return level->edgeDown;
  }
  }
  return NULL;
}

/**
 * End of Helper functions
 */

bool GameWorldChangeLevelAtDoor(GameWorld *gameWorld,
                                const char *targetLevelPath, int targetDoorX,
                                int targetDoorY, int spawnOffsetX,
                                int spawnOffSetY) {
  Level newLevel = {0};
  if (!LevelLoadFromFile(&newLevel, path)) {
    return false;
  }
  if (LevelFindDoorAtTile(&newLevel, targeDoorX, targetDoorY) == NULL) {
    LevelFree(&newLevel);
    return false;
  }
  GameWorldCommitTransition(gameWorld, &newLevel, path);
  PlacePlayerAtDoorSpawn(&gameWorld->player, targetDoorX + spawnOffsetX,
                         targetDoorY + spawnOffSetY);
  CameraSnapToPlayer(&gameWorld->camera, &gameWorld->player, &gameWorld->level);
  return true;
}
bool GameWorldHandleDoorUse(GameWorld *gameWorld, const Input *input) {
  if (!input->isUseJustPressed) {
    return false;
  }

  for (int i = 0; i < gameWorld->level.doorCount; i++) {
    const DoorTransition *doorTransition = &gameWorld->level.doors[i];
    if (!DoorOverlapsPlayer(doorTransition, &gameWorld->player)) {
      continue;
    }
    char targetLevelPath[MAX_LEVEL_PATH_LENGTH + 32];
    ResolveLevelPath(targetLevelPath, sizeof(targetLevelPath),
                     gameWorld->currentLevelPath,
                     doorTransition->targetLevelPath);
    int targetDoorX = doorTransition->targetDoorTileX;
    int targetDoorY = doorTransition->targetDoorTileY;
    int spawnOffsetTileX = doorTransition->spawnOffsetTileX;
    int spawnOffsetTileY = doorTransition->spawnOffsetTileY;
    /**
     * door is now unsafe to read, the level swap invalidates it
     */
    return GameWorldChangeLevelAtDoor(gameWorld, targetLevelPath, targetDoorX,
                                      targetDoorY, spawnOffsetTileX,
                                      spawnOffsetTileY);
  }
  return false;
}

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
  if (GameWorldHandleDoorUse(gameWorld, input)) {
    return;
  }
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
                     SDL_Texture *levelEnemyTexture, SDL_Texture *doorTexture) {
  GraphicsRenderLevel(&gameWorld->level, renderer, &gameWorld->camera);
  GraphicsRenderPlayer(&gameWorld->player, renderer, &gameWorld->camera);
  ProjectileRenderAll(gameWorld->projectiles, renderer, gameWorld->camera.x,
                      gameWorld->camera.y, projectileTexture);
  GraphicsRenderDoors(&gameWorld->level, renderer, &gameWorld->camera,
                      doorTexture);
  LevelEnemyRenderAll(gameWorld->level.levelEnemies,
                      LevelGetEnemyCount(&gameWorld->level), renderer,
                      &gameWorld->camera, levelEnemyTexture);
}

bool GameWorldChangeLevelAtEdge(GameWorld *gameWorld, const char *path,
                                Direction exitDirection) {
  Level newLevel = {0};
  if (!LevelLoadFromFile(&newLevel, Path)) {
    return false;
  }
  GameWorldCommitTransition(gameWorld, &newLevel, path);
  PlacePlayerForEdgeArrival(&gameWorld->player, &gameWorld->level,
                            exitDirection);
  CameraSnapToPlayer(&gameWorld->camera, &gameWorld->player, &gameWorld->level);
  return true;
}