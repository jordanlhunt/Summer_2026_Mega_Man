#include "level.h"
#include "config.h"
#include "levelEnemy.h"
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
  LevelEnemy newEnemies[MAX_LEVEL_ENEMIES] = {0};
  int newEnemyCount = 0;
  bool spawnFound = false;
  float spawnX = 0.0f;
  float spawnY = 0.0f;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      char fileCharacter;
      if (fscanf(levelFile, " %c", &fileCharacter) != 1) {
        SDL_Log("Level ended early at tile (%d, %d): %s", x, y, filePath);
        free(newTiles);
        fclose(levelFile);
        return false;
      }
      if (fileCharacter != '0' && fileCharacter != '1' &&
          fileCharacter != '6' && fileCharacter != '7') {
        SDL_Log("Invalid tile '%c' at (%d, %d): %s", fileCharacter, x, y,
                filePath);
        free(newTiles);
        fclose(levelFile);
        return false;
      }
      size_t index = (size_t)y * (size_t)width + (size_t)x;
      if (fileCharacter == '6') {
        if (spawnFound) {
          SDL_Log("Level contains multiple player spawns: %s", filePath);
          free(newTiles);
          fclose(levelFile);
          return false;
        }
        spawnFound = true;
        spawnX = (float)(x * TILE_SIZE);
        spawnY = (float)(y * TILE_SIZE);
        newTiles[index] = 0;
      } else if (fileCharacter == '7') {
        if (newEnemyCount >= MAX_LEVEL_ENEMIES) {
          SDL_Log("Too many enemies in level at (%d, %d): %s", x, y, filePath);
          free(newTiles);
          fclose(levelFile);
          return false;
        }

        LevelEnemyInitialize(&newEnemies[newEnemyCount], (float)(x * TILE_SIZE),
                             (float)(y * TILE_SIZE), LEVELENEMY_STATE_FLYING);

        newEnemyCount += 1;
        newTiles[index] = 0;
      } else {
        newTiles[index] = (unsigned char)(fileCharacter - '0');
      }
    }
  }
  fclose(levelFile);
  if (!spawnFound) {
    SDL_Log("Level does not contain a player spawn: %s", filePath);
    free(newTiles);
    return false;
  }
  free(level->tiles);
  level->tiles = newTiles;
  level->width = width;
  level->height = height;
  level->playerSpawnX = spawnX;
  level->playerSpawnY = spawnY;
  level->hasPlayerSpawn = true;
  return true;
}

int LevelGetEnemyCount(const Level *level) {
  if (level != NULL) {
    return level->levelEnemiesCount;
  } else {
    return 0;
  }
}

const LevelEnemy *LevelGetEnemyAt(Level *level, int index) {
  if (!level || index < 0 || index >= level->levelEnemiesCount) {
    return NULL;
  }
  return &level->levelEnemies[index];
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
  return level != NULL ? level->width : 0;
}

int LevelGetHeight(const Level *level) {
  return level != NULL ? level->height : 0;
}

float LevelGetWidthPixels(const Level *level) {
  return (float)(LevelGetWidth(level) * TILE_SIZE);
}

float LevelGetHeightPixels(const Level *level) {
  return (float)(LevelGetHeight(level) * TILE_SIZE);
}

float LevelGetPlayerSpawnX(const Level *level) {
  return level != NULL ? level->playerSpawnX : 0.0f;
}

float LevelGetPlayerSpawnY(const Level *level) {
  return level != NULL ? level->playerSpawnY : 0.0f;
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

bool LevelIsSolidTile(const Level *level, int tileX, int tileY) {
  return LevelGetTile(level, tileX, tileY) != 0;
}