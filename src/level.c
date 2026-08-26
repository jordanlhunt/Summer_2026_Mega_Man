#include "level.h"
void LevelFree(Level *level) {
  if (level) {
    free(level->tiles);
    level->tiles = NULL;
    level->width = 0;
    level->height = 0;
    level->hasPlayerSpawn = false;
  }
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
  if ((size_t)width > SIZE_MAX / (size_t)height) {
    SDL_Log("Level dimensions overflow: %d x %d", width, height);
    fclose(levelFile);
    return false;
  }
  unsigned char *newTiles = calloc(tileCount, sizeof(*newTiles));
  if (newTiles == NULL) {
    SDL_Log("Could not allocate memory for level: %s", filePath);
    fclose(levelFile);
    return false;
  }
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
          fileCharacter != '6') {
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