#include "gameworld.h"
#include "collision.h"
#include "config.h"

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
  player->invincibleFramesTimer = 0.0f;
  player->knockbackTimer = 0.0f;
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
  const char *lastSlash = strrchr(currentLevelPath, '/');
  if (lastSlash == NULL) {
    snprintf(out, outSize, "%s", targetFileName);
    return;
  }
  int directoryLength = (int)(lastSlash - currentLevelPath + 1);
  snprintf(out, outSize, "%.*s%s", directoryLength, currentLevelPath,
           targetFileName);
}

static void GameWorldCommitTransition(GameWorld *gameWorld, Level *newLevel,
                                      const char *pathToNewLevel) {
  LevelFree(&gameWorld->level);
  gameWorld->level = *newLevel;
  PlayerResetForRoomTransition(&gameWorld->player);
  memset(gameWorld->projectiles, 0, sizeof(gameWorld->projectiles));
  strncpy(gameWorld->currentLevelPath, pathToNewLevel,
          MAX_LEVEL_PATH_LENGTH - 1);
  gameWorld->currentLevelPath[MAX_LEVEL_PATH_LENGTH - 1] = '\0';
}
static void PlacePlayerForEdgeArrival(Player *player, const Level *level,
                                      Direction exitDirection) {
  float roomWidthInPixels = LevelGetWidthPixels(level);
  float roomHeightInPixels = LevelGetHeightPixels(level);
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

static void GameWorldHandleSpikeContact(GameWorld *gameWorld) {
  const float inset = 3.0f;
  if (!CollisionFindTileOfType(&gameWorld->level,
                               gameWorld->player.entity.x + inset,
                               gameWorld->player.entity.y + inset,
                               gameWorld->player.entity.width - inset * 2.0f,
                               gameWorld->player.entity.height - inset * 2.0f,
                               TILE_SPIKE, NULL, NULL)) {
    return;
  }
  // Insta-kill the player just like old Mega-Man does
  gameWorld->player.hitPoints = 0;
}

static bool GameWorldHandlePitfall(GameWorld *gameWorld) {

  float killY = LevelGetHeightPixels(&gameWorld->level) + PITFALL_DEATH_MARGIN;
  if (gameWorld->player.entity.y <= killY) {
    return false;
  }
  gameWorld->player.hitPoints = 0;
  return true;
}

static bool GameWorldHandleEdgeTransition(GameWorld *gameWorld) {
  const Level *level = &gameWorld->level;
  float roomWidthInPixels = LevelGetWidthPixels(level);
  float roomHeightinPixels = LevelGetHeightPixels(level);
  const float margin = 2.0f;
  const Player *player = &gameWorld->player;
  Direction direction;
  if (player->entity.x <= margin && player->entity.velocityX < 0.0f) {
    direction = LEFT;
  } else if (player->entity.x + player->entity.width >=
                 roomWidthInPixels - margin &&
             player->entity.velocityX > 0.0f) {
    direction = RIGHT;
  } else if (player->entity.y <= margin && player->entity.velocityY < 0.0f) {
    direction = UP;
  } else if (player->entity.y + player->entity.height >=
                 roomHeightinPixels - margin &&
             player->entity.velocityY > 0.0f) {
    direction = DOWN;
  } else {
    return false;
  }
  const char *target = EdgeTargetFor(level, direction);
  if (target == NULL || target[0] == '\0') {
    return false;
  }
  char resolvedLevelPath[MAX_LEVEL_PATH_LENGTH];
  ResolveLevelPath(resolvedLevelPath, sizeof(resolvedLevelPath),
                   gameWorld->currentLevelPath, target);
  return GameWorldChangeLevelAtEdge(gameWorld, resolvedLevelPath, direction);
}

// If the player collides with an enemy take damage
static void GameWorldHandleEnemyContact(GameWorld *gameWorld) {
  int enemyCount = LevelGetEnemyCount(&gameWorld->level);
  for (int i = 0; i < enemyCount; i++) {
    LevelEnemy *enemy = &gameWorld->level.levelEnemies[i];
    if (!enemy->entity.isActive) {
      continue;
    }
    if (!EntityOverlaps(&gameWorld->player.entity, &enemy->entity)) {
      continue;
    }
    PlayerTakeDamage(&gameWorld->player, 1, enemy->entity.x);
    break;
  }
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
  PlayerResetForRoomTransition(&gameWorld->player);
  // Initialize the player projectile pool
  memset(gameWorld->projectiles, 0, sizeof(gameWorld->projectiles));
  // Remember currrent location so transitions can resolve relative path
  strncpy(gameWorld->currentLevelPath, levelPath, MAX_LEVEL_PATH_LENGTH - 1);
  gameWorld->currentLevelPath[MAX_LEVEL_PATH_LENGTH - 1] = '\0';
  return true;
}

void GameWorldShutdown(GameWorld *gameWorld) { LevelFree(&gameWorld->level); }
void GameWorldUpdate(GameWorld *gameWorld, const Input *input,
                     float deltaTime) {
  PlayerUpdate(&gameWorld->player, input, &gameWorld->level, deltaTime);
  GameWorldHandleSpikeContact(gameWorld);
  GameWorldHandleEnemyContact(gameWorld);
  if (GameWorldHandleEdgeTransition(gameWorld)) {
    return;
  }
  if (GameWorldHandlePitfall(gameWorld)) {
    return;
  }
  ProjectileHandlePlayerShooting(gameWorld->projectiles, &gameWorld->player,
                                 input->isShootJustPressed);
  ProjectileUpdateAll(gameWorld->projectiles, &gameWorld->level, deltaTime);
  LevelEnemyUpdateAll(gameWorld->level.levelEnemies,
                      LevelGetEnemyCount(&gameWorld->level), deltaTime);
  HandleProjectileEntityCollision(gameWorld->projectiles, &gameWorld->level);
  HealthPickupUpdateAll(gameWorld->level.healthPickups,
                        gameWorld->level.healthPickupCount, &gameWorld->level,
                        deltaTime);
  HealthPickupCollectAll(gameWorld->level.healthPickups,
                         gameWorld->level.healthPickupCount,
                         &gameWorld->player);
  float targetCameraX = gameWorld->player.entity.x - SCREEN_WIDTH / 2.0f;
  float targetCameraY = gameWorld->player.entity.y - SCREEN_HEIGHT / 2.0f;
  CameraUpdate(&gameWorld->camera, targetCameraX, targetCameraY,
               &gameWorld->level, deltaTime);
}
void GameWorldRender(const GameWorld *gameWorld, SDL_Renderer *renderer,
                     SDL_Texture *projectileTexture,
                     SDL_Texture *levelEnemyTexture,
                     SDL_Texture *healthPickupTexture) {
  GraphicsRenderLevel(&gameWorld->level, renderer, &gameWorld->camera);
  HealthPickupRenderAll(gameWorld->level.healthPickups,
                        gameWorld->level.healthPickupCount, renderer,
                        &gameWorld->camera, healthPickupTexture);
  GraphicsRenderPlayer(&gameWorld->player, renderer, &gameWorld->camera);

  ProjectileRenderAll(gameWorld->projectiles, renderer, gameWorld->camera.x,
                      gameWorld->camera.y, projectileTexture);
  LevelEnemyRenderAll(gameWorld->level.levelEnemies,
                      LevelGetEnemyCount(&gameWorld->level), renderer,
                      &gameWorld->camera, levelEnemyTexture);
}
bool GameWorldChangeLevelAtEdge(GameWorld *gameWorld, const char *path,
                                Direction exitDirection) {
  Level newLevel = {0};
  if (!LevelLoadFromFile(&newLevel, path)) {
    SDL_Log("Edge transition failed: could not load %s", path);
    return false;
  }
  GameWorldCommitTransition(gameWorld, &newLevel, path);
  PlacePlayerForEdgeArrival(&gameWorld->player, &gameWorld->level,
                            exitDirection);
  CameraSnapToPlayer(&gameWorld->camera, &gameWorld->player, &gameWorld->level);
  return true;
}
