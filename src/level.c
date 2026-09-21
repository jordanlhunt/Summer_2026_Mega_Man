#include "level.h"
#include "config.h"
#include "levelEnemy.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct LevelParseState {
  bool *spawnPlayerFound;
  DoorTransition *doors;
  float *playerSpawnX;
  float *playerSpawnY;
  int *doorCount;
  int *levelEnemiesCount;
  int width;
  LevelEnemy *levelEnemies;
  unsigned char *tiles;
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
      fileCharacter != TILE_DOOR_CHAR) {
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
  } else if (fileCharacter == '8') {
    if (*state->doorCount >= MAX_DOORS) {
      SDL_Log("Too many doors at (%d, %d): %s", x, y, filePath);
      return false;
    }
    DoorTransition *doorTransition = &state->doors[*state->doorCount];
    doorTransition->tileX = x;
    doorTransition->tileY = y;
    state->doors[*state->doorCount].targetLevelPath[0] = '\0';
    state->doors[*state->doorCount].targetDoorTileX = -1;
    state->doors[*state->doorCount].targetDoorTileY = -1;
    state->doors[*state->doorCount].spawnOffsetTileX = 0;
    state->doors[*state->doorCount].spawnOffsetTileY = 0;
    *state->doorCount += 1;
    // Make door tiles walkable
    state->tiles[index] = 0;
  } else {
    state->tiles[index] = (unsigned char)(fileCharacter - TILE_EMPTY_CHAR);
  }
  return true;
}
static bool LevelParseTransitions(File *newRoom, Level *newLevel,
                                  LevelEnemy *levelEnemies,
                                  int *levelEnemyCount,
                                  DoorTransition *doorTransitions,
                                  int *doorCount, const char *filePath) {
  char transitionType[16];
  while (fscanf(targetRoom, " %15s", transitionType) == 1) {
    if (strcmp(transitionType, TRANSITION_TYPE_DOOR) == 0) {
      int doorTileX, doorTileY, targetDoorTileX, targetDoorTileY, spawnOffsetX,
          spawnOffsetY;
      char targetLevelFileName[MAX_LEVEL_PATH_LENGTH];
      if (fscanf(newRoom, " %d %d %127s %d %d %d %d", &doorTileX, &doorTileY,
                 targetLevelFileName, &targetDoorTileX, &targetDoorTileY,
                 &spawnOffsetX, &spawnOffsetY) != 7) {
        SDL_Log("Malformed door line in %s", filePath);
        return false;
      }
      if (*doorCount >= MAX_DOORS) {
        return false;
      }
      DoorTransition *door = &doorTransitions[*doorCount];
      door->tileX = doorTileX;
      door->tileY = doorTileY;
      strncpy(door->targetLevelPath, targetLevelFileName,
              MAX_LEVEL_PATH_LENGTH - 1);
      door->targetDoorTileX = targetDoorTileX;
      door->targetDoorTileY = targetDoorTileY;
      door->spawnOffsetTileX = spawnOffsetX;
      door->spawnOffsetTileY = spawnOffsetY;
      (*doorCount) += 1;
    } else if (strcmp(transitionType, "edge") == 0) {
      char directionType[16], targetLevelFileName[MAX_LEVEL_PATH_LENGTH];
      if (fscanf(newRoom, " %15s %127s", directionType, targetLevelFileName) !=
          2) {
        SDL_Log("Malformed edge line in %s", filePath);
        return false;
      }
      char *destination = NULL;
      if (strcmp(directionType, "left") == 0) {
        destination = newLevel->edgeLeft;
      } else if (strcmp(directionType, "right") == 0) {
        destination = newLevel->edgeRight;
      } else if (strcmp(directionType, "up") == 0) {
        destination = newLevel->edgeUp;
      } else if (strcmp(directionType, "down") == 0) {
        destination = newLevel->edgeDown;
      }
    } else {
      SDL_Log("Unknown edge direction '%s'", directionType);
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
                                  const DoorTransition newDoors[],
                                  int newDoorCount) {
  free(level->tiles);
  level->tiles = tiles;
  level->width = width;
  level->height = height;
  level->hasPlayerSpawn = spawnFound;
  level->playerSpawnX = spawnX;
  level->playerSpawnY = spawnY;
  memcpy(level->levelEnemies, newEnemies, sizeof(level->levelEnemies));
  level->levelEnemiesCount = newEnemyCount;
  memcpy(level->doors, newDoors, sizeof(level->doors));
  level->doorCount = newDoorCount;
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

  size_t tileCount = (size_t)width * (size_t)height;
  unsigned char *newTiles = calloc(tileCount, sizeof(*newTiles));
  if (newTiles == NULL) {
    SDL_Log("Could not allocate memory for level: %s", filePath);
    fclose(levelFile);
    return false;
  }

  DoorTransition newDoors[MAX_DOORS] = {0};
  LevelEnemy newEnemies[MAX_LEVEL_ENEMIES] = {0};
  int newDoorCount = 0;
  int newEnemyCount = 0;
  bool spawnFound = false;
  float spawnX = 0.0f;
  float spawnY = 0.0f;
  LevelParseState parseState = {
      .tiles = newTiles,
      .width = width,
      .levelEnemies = newEnemies,
      .levelEnemiesCount = &newEnemyCount,
      .spawnPlayerFound = &spawnFound,
      .playerSpawnX = &spawnX,
      .playerSpawnY = &spawnY,
      .doors = newDoors,
      .doorCount = &newDoorCount,
  };

  if (!LevelParseGrid(levelFile, &parseState, width, height, filePath)) {
    free(newTiles);
    fclose(levelFile);
    return false;
  }

  if (!LevelParseDoorTransitions(levelFile, newDoors, newDoorCount, filePath)) {
    free(newTiles);
    fclose(levelFile);
    return false;
  }

  LevelCommitLoadedData(level, newTiles, width, height, spawnFound, spawnX,
                        spawnY, newEnemies, newEnemyCount, newDoors,
                        newDoorCount);
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
const DoorTransition *LevelFindDoorAtTile(const Level *level, int tileX,
                                          int tileY) {
  if (level == NULL) {
    return NULL;
  }
  for (int i = 0; i < level->doorCount; i++) {
    if (level->doors[i].tileX == tileX && level->doors[i].tileY == tileY) {
      return &level->doors[i];
    }
  }
  return NULL;
}