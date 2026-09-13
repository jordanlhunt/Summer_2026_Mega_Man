#include "level.h"
#include "config.h"
#include "levelEnemy.h"
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
    state->doors[*state->doorCount].targetLevelPath[0] = '\0';
    state->doors[*state->doorCount].targetDoorTileX = -1;
    state->doors[*state->doorCount].targetDoorTileY = -1;
    state->doors[*state->doorCount].spawnOffsetTileX = 0;
    state->doors[*state->doorCount].spawnOffsetTileY = 0;
    state->doorCount += 1;
    // Make door tiles walkable
    state->tiles[index] = 0;

  }

  else {
    state->tiles[index] = (unsigned char)(fileCharacter - TILE_EMPTY_CHAR);
  }
  return true;
}
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
  int width;
  int height;
  if (fscanf(levelFile, "%d %d", &width, &height) != 2) {
    SDL_Log("Could not read level dimensions from %s", filePath);
    fclose(levelFile);
    return false;
  }
  if (width <= 0 || height <= 0) {
    SDL_Log("Invalid level dimensions in %s: %d x %d", filePath, width, height);
    fclose(levelFile);
    return false;
  }
  if ((size_t)width > SIZE_MAX / (size_t)height) {
    SDL_Log("Level dimensions are too large: %s", filePath);
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
      .doorCount = newDoorCount,
  };
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      char fileCharacter;
      if (fscanf(levelFile, " %c", &fileCharacter) != 1) {
        SDL_Log("Level ended early at tile (%d, %d): %s", x, y, filePath);
        free(newTiles);
        fclose(levelFile);
        return false;
      }
      if (!LevelParseTileChar(&parseState, fileCharacter, x, y, filePath)) {
        free(newTiles);
        fclose(levelFile);
        return false;
      }
    }
  }
  free(level->tiles);
  level->tiles = newTiles;
  level->width = width;
  level->height = height;
  level->hasPlayerSpawn = spawnFound;
  level->playerSpawnX = spawnX;
  level->playerSpawnY = spawnY;
  memcpy(level->levelEnemies, newEnemies, sizeof(newEnemies));
  level->levelEnemiesCount = newEnemyCount;
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
