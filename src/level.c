#include "level.h"
typedef struct LevelParseState {
  bool *spawnPlayerFound;

  float *playerSpawnX;
  float *playerSpawnY;

  int *levelEnemiesCount;
  int width;
  LevelEnemy *levelEnemies;
  unsigned char *tiles;
  HealthPickup *healthPickups;
  int *healthPickupCount;
} LevelParseState;
/**
 * Helper Functions
 */
static bool LevelParseTileChar(LevelParseState *state, char fileCharacter,
                               int x, int y, const char *filePath) {
  if (fileCharacter != TILE_EMPTY_CHAR && fileCharacter != TILE_SOLID_CHAR &&
      fileCharacter != TILE_ONE_WAY_CHAR &&
      fileCharacter != TILE_BREAKABLE_CHAR &&
      fileCharacter != TILE_PLAYER_SPAWN_CHAR &&
      fileCharacter != TILE_ENEMY_SPAWN_CHAR &&
      fileCharacter != TILE_SPIKE_CHAR && fileCharacter != TILE_HEALTH_CHAR) {
    SDL_Log("Invalid tile '%c' at (%d, %d): %s", fileCharacter, x, y, filePath);
    return false;
  }
  size_t index = (size_t)y * (size_t)state->width + (size_t)x;
  if (fileCharacter == TILE_PLAYER_SPAWN_CHAR) {
    if (*state->spawnPlayerFound) {
      SDL_Log("Level contains multiple player spawns: %s", filePath);
      return false;
    }
    *state->spawnPlayerFound = true;
    *state->playerSpawnX = (float)(x * TILE_SIZE);
    *state->playerSpawnY = (float)(y * TILE_SIZE);
    state->tiles[index] = 0;
  } else if (fileCharacter == TILE_ENEMY_SPAWN_CHAR) {
    if (*state->levelEnemiesCount >= MAX_LEVEL_ENEMIES) {
      SDL_Log("Too many enemies in level at (%d, %d): %s", x, y, filePath);
      return false;
    }
    LevelEnemyInitialize(&state->levelEnemies[*state->levelEnemiesCount],
                         (float)(x * TILE_SIZE), (float)(y * TILE_SIZE),
                         LEVELENEMY_STATE_FLYING);
    *state->levelEnemiesCount += 1;
    state->tiles[index] = 0;
  } else if (fileCharacter == TILE_SPIKE_CHAR) {
    state->tiles[index] = TILE_SPIKE;
  } else if (fileCharacter == TILE_HEALTH_CHAR) {
    if (*state->healthPickupCount >= MAX_HEALTH_PICKUPS) {
      SDL_Log("Too many health pickups at (%d, %d): %s", x, y, filePath);
      return false;
    }
    float pickupX =
        (float)(x * TILE_SIZE) + (TILE_SIZE - HEALTH_PICKUP_SIZE) * 0.5f;
    float pickupY =
        (float)(y * TILE_SIZE) + (TILE_SIZE - HEALTH_PICKUP_SIZE) * 0.5f;
    HealthPickupInitialize(&state->healthPickups[*state->healthPickupCount],
                           pickupX, pickupY);
    *state->healthPickupCount += 1;
    state->tiles[index] = TILE_EMPTY;
  } else {
    state->tiles[index] = (unsigned char)(fileCharacter - TILE_EMPTY_CHAR);
  }
  return true;
}
static bool LevelParseTransitions(FILE *newRoom, char *edgeLeft,
                                  char *edgeRight, char *edgeUp, char *edgeDown,
                                  const char *filePath) {
  char transitionType[16];
  while (fscanf(newRoom, " %15s", transitionType) == 1) {
    if (strcmp(transitionType, TRANSITION_TYPE_EDGE) == 0) {
      char directionType[16];
      char targetLevelFileName[MAX_LEVEL_PATH_LENGTH];
      if (fscanf(newRoom, " %15s %127s", directionType, targetLevelFileName) !=
          2) {
        SDL_Log("Malformed edge line in %s", filePath);
        return false;
      }
      char *destination = NULL;
      if (strcmp(directionType, "left") == 0) {
        destination = edgeLeft;
      } else if (strcmp(directionType, "right") == 0) {
        destination = edgeRight;
      } else if (strcmp(directionType, "up") == 0) {
        destination = edgeUp;
      } else if (strcmp(directionType, "down") == 0) {
        destination = edgeDown;
      } else {
        SDL_Log("Unknown edge direction '%s' in %s", directionType, filePath);
        return false;
      }
      strncpy(destination, targetLevelFileName, MAX_LEVEL_PATH_LENGTH - 1);
      destination[MAX_LEVEL_PATH_LENGTH - 1] = '\0';
    } else {
      SDL_Log("Unknown transition type '%s' in %s", transitionType, filePath);
      return false;
    }
  }
  return true;
}
static bool LevelReadDimensions(FILE *levelFile, int *outWidth, int *outHeight,
                                const char *filePath) {
  if (fscanf(levelFile, "%d %d", outWidth, outHeight) != 2) {
    SDL_Log("Could not read level dimensions from %s", filePath);
    return false;
  }
  if (*outWidth <= 0 || *outHeight <= 0) {
    SDL_Log("Invalid level dimensions in %s: %d x %d", filePath, *outWidth,
            *outHeight);
    return false;
  }
  if ((size_t)*outWidth > SIZE_MAX / (size_t)*outHeight) {
    SDL_Log("Level dimensions are too large: %s", filePath);
    return false;
  }
  return true;
}
static bool LevelParseGrid(FILE *levelFile, LevelParseState *parseState,
                           int width, int height, const char *filePath) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      char fileCharacter;
      if (fscanf(levelFile, " %c", &fileCharacter) != 1) {
        SDL_Log("Level ended early at tile (%d, %d): %s", x, y, filePath);
        return false;
      }
      if (!LevelParseTileChar(parseState, fileCharacter, x, y, filePath)) {
        return false;
      }
    }
  }
  return true;
}
static void LevelCommitLoadedData(Level *level, unsigned char *tiles, int width,
                                  int height, bool spawnFound, float spawnX,
                                  float spawnY, const LevelEnemy newEnemies[],
                                  int newEnemyCount,
                                  const HealthPickup newHealthPickups[],
                                  int newHealthPickupCount,
                                  const char *edgeLeft, const char *edgeRight,
                                  const char *edgeUp, const char *edgeDown) {
  free(level->tiles);
  level->tiles = tiles;
  level->width = width;
  level->height = height;
  level->hasPlayerSpawn = spawnFound;
  level->playerSpawnX = spawnX;
  level->playerSpawnY = spawnY;
  memcpy(level->levelEnemies, newEnemies, sizeof(level->levelEnemies));
  level->levelEnemiesCount = newEnemyCount;
  memcpy(level->healthPickups, newHealthPickups, sizeof(level->healthPickups));
  level->healthPickupCount = newHealthPickupCount;
  strncpy(level->edgeLeft, edgeLeft, MAX_LEVEL_PATH_LENGTH - 1);
  strncpy(level->edgeRight, edgeRight, MAX_LEVEL_PATH_LENGTH - 1);
  strncpy(level->edgeUp, edgeUp, MAX_LEVEL_PATH_LENGTH - 1);
  strncpy(level->edgeDown, edgeDown, MAX_LEVEL_PATH_LENGTH - 1);
  level->edgeLeft[MAX_LEVEL_PATH_LENGTH - 1] = '\0';
  level->edgeRight[MAX_LEVEL_PATH_LENGTH - 1] = '\0';
  level->edgeUp[MAX_LEVEL_PATH_LENGTH - 1] = '\0';
  level->edgeDown[MAX_LEVEL_PATH_LENGTH - 1] = '\0';
}
/**
 * End of Helper functions
 */
void LevelFree(Level *level) {
  if (level == NULL) {
    return;
  }
  free(level->tiles);
  *level = (Level){0};
}

bool LevelAddHealthPickup(Level *level, float x, float y) {
  if (level == NULL) {
    return false;
  }
  // Reuse the slots if possible
  for (int i = 0; i < level->healthPickupCount; i++) {
    if (!level->healthPickups[i].entity.isActive) {
      HealthPickupInitialize(&level->healthPickups[i], x, y);
      return;
    }
  }
  if (level->healthPickupCount >= MAX_HEALTH_PICKUPS) {
    return false;
  }
  HealthPickupInitialize(&level->healthPickups[level->healthPickupCount], x, y);
  level->healthPickupCount += 1;
  return true;
}

bool LevelLoadFromFile(Level *level, const char *filePath) {
  if (level == NULL || filePath == NULL) {
    return false;
  }
  FILE *levelFile = fopen(filePath, "r");
  if (levelFile == NULL) {
    SDL_Log("Could not open level file '%s': %s", filePath, SDL_GetError());
    return false;
  }
  int width, height;
  if (!LevelReadDimensions(levelFile, &width, &height, filePath)) {
    fclose(levelFile);
    return false;
  }
  if (width != LEVEL_DIMENSIONS_X || height != LEVEL_DIMENSIONS_Y) {
    SDL_Log("Level '%s' is %dx%d, expected %dx%d. Standarize the file.",
            filePath, width, height, LEVEL_DIMENSIONS_X, LEVEL_DIMENSIONS_Y);
    fclose(levelFile);
    return false;
  }
  size_t tileCount = (size_t)width * (size_t)height;
  unsigned char *newTiles = calloc(tileCount, sizeof(*newTiles));
  if (newTiles == NULL) {
    SDL_Log("Could not allocate memory for level: %s", filePath);
    fclose(levelFile);
    return false;
  }
  HealthPickup newHealthPickups[MAX_HEALTH_PICKUPS] = {0};
  int newHealthPickupCount = 0;
  LevelEnemy newEnemies[MAX_LEVEL_ENEMIES] = {0};
  char edgeLeft[MAX_LEVEL_PATH_LENGTH] = {0};
  char edgeRight[MAX_LEVEL_PATH_LENGTH] = {0};
  char edgeUp[MAX_LEVEL_PATH_LENGTH] = {0};
  char edgeDown[MAX_LEVEL_PATH_LENGTH] = {0};

  int newEnemyCount = 0;
  bool spawnFound = false;
  float spawnX = 0.0f;
  float spawnY = 0.0f;
  LevelParseState parseState = {.tiles = newTiles,
                                .width = width,
                                .levelEnemies = newEnemies,
                                .levelEnemiesCount = &newEnemyCount,
                                .spawnPlayerFound = &spawnFound,
                                .playerSpawnX = &spawnX,
                                .playerSpawnY = &spawnY,
                                .healthPickups = newHealthPickups,
                                .healthPickupCount = &newHealthPickupCount};
  if (!LevelParseGrid(levelFile, &parseState, width, height, filePath)) {
    free(newTiles);
    fclose(levelFile);
    return false;
  }
  if (!LevelParseTransitions(levelFile, edgeLeft, edgeRight, edgeUp, edgeDown,
                             filePath)) {
    free(newTiles);
    fclose(levelFile);
    return false;
  }
  LevelCommitLoadedData(level, newTiles, width, height, spawnFound, spawnX,
                        spawnY, newEnemies, newEnemyCount, newHealthPickups,
                        newHealthPickupCount, edgeLeft, edgeRight, edgeUp,
                        edgeDown);
  fclose(levelFile);
  return true;
}
int LevelGetEnemyCount(const Level *level) {
  if (level != NULL) {
    return level->levelEnemiesCount;
  } else {
    return 0;
  }
}
bool LevelAddEnemy(Level *level, float x, float y,
                   LevelEnemyState initialState) {
  if (level == NULL) {
    return false;
  }
  if (level->levelEnemiesCount < 0 ||
      level->levelEnemiesCount >= MAX_LEVEL_ENEMIES) {
    return false;
  }
  LevelEnemy *enemy = &level->levelEnemies[level->levelEnemiesCount];
  LevelEnemyInitialize(enemy, x, y, initialState);
  level->levelEnemiesCount += 1;
  return true;
}
int LevelGetWidth(const Level *level) {
  if (level != NULL) {
    return level->width;
  } else {
    return 0;
  }
}
int LevelGetHeight(const Level *level) {
  if (level != NULL) {
    return level->height;
  } else {
    return 0;
  }
}
float LevelGetWidthPixels(const Level *level) {
  return (float)(LevelGetWidth(level) * TILE_SIZE);
}
float LevelGetHeightPixels(const Level *level) {
  return (float)(LevelGetHeight(level) * TILE_SIZE);
}
float LevelGetPlayerSpawnX(const Level *level) {
  if (level != NULL) {
    return level->playerSpawnX;
  } else {
    return 0.0f;
  }
}
float LevelGetPlayerSpawnY(const Level *level) {
  if (level != NULL) {
    return level->playerSpawnY;
  } else {
    return 0.0f;
  }
}
unsigned char LevelGetTile(const Level *level, int tileX, int tileY) {
  if (level == NULL || level->tiles == NULL) {
    return 0;
  }
  if (tileX < 0 || tileX >= level->width || tileY < 0 ||
      tileY >= level->height) {
    return 0;
  }
  return level->tiles[tileY * level->width + tileX];
}
