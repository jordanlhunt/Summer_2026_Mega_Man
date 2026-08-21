#include "level.h"
#include "game.h"
void LevelFree(Level *level) {
  if (level) {
    free(level->tiles);
    level->tiles = NULL;
    level->width = 0;
    level->height = 0;
    level->hasPlayerSpawn = false;
  }
}
void LevelRender(const Level *level, SDL_Renderer *renderer, float cameraX,
                 float cameraY) {
  int startX = (int)floorf(cameraX / TILE_SIZE) - 1;
  int startY = (int)floorf(cameraY / TILE_SIZE) - 1;
  int endX = startX + (SCREEN_WIDTH / TILE_SIZE) + 3;
  int endY = startY + (SCREEN_HEIGHT / TILE_SIZE) + 3;
  for (int y = startY; y < endY; y++) {
    for (int x = startX; x < endX; x++) {
      if (x < 0 || y < 0) {
        continue;
      }
      if (x >= level->width || y >= level->height) {
        continue;
      }
      if (level->tiles[y * level->width + x] == 0) {
        continue;
      }
      SDL_FRect tile = {.x = x * TILE_SIZE - cameraX,
                        .y = y * TILE_SIZE - cameraY,
                        .w = TILE_SIZE,
                        .h = TILE_SIZE};
      bool isFloor = (y >= level->height - 3);
      bool isWall = (x == 0 || x == level->width - 1);
      if (y + 1 < level->height &&
          level->tiles[(y + 1) * level->width + x] == 0 && isFloor == false) {
        SDL_SetRenderDrawColor(renderer, 80, 70, 60, 255);
      } else if (isWall) {
        SDL_SetRenderDrawColor(renderer, 60, 55, 50, 255);
      } else {
        SDL_SetRenderDrawColor(renderer, 45, 40, 35, 255);
      }
      SDL_RenderFillRect(renderer, &tile);
    }
  }
}
bool LevelLoadFromFile(Level *level, const char *filePath) {
  if (level == NULL || filePath == NULL) {
    return false;
  }
  // If this Level already owns tiles, release them first.
  free(level->tiles);
  level->tiles = NULL;
  level->width = 0;
  level->height = 0;
  level->hasPlayerSpawn = false;
  FILE *levelFile = fopen(filePath, "r");
  if (levelFile == NULL) {
    SDL_Log("Could not open level file: %s", filePath);
    return false;
  }
  int width = 0;
  int height = 0;
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
  size_t tileCount = (size_t)width * (size_t)height;
  unsigned char *tiles = calloc(tileCount, sizeof(*tiles));
  if (tiles == NULL) {
    SDL_Log("Could not allocate memory for level: %s", filePath);
    fclose(levelFile);
    return false;
  }
  bool isPlayerSpawnFound = false;
  float spawnX = 0.0f;
  float spawnY = 0.0f;
  char fileCharacter;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (fscanf(levelFile, " %c", &fileCharacter) != 1) {
        SDL_Log("Level file ended early at tile (%d, %d): %s", x, y, filePath);
        free(tiles);
        fclose(levelFile);
        return false;
      }
      if (fileCharacter != '0' && fileCharacter != '1' &&
          fileCharacter != '6') {
        SDL_Log("Invalid tile '%c' at tile (%d, %d): %s", fileCharacter, x, y,
                filePath);
        free(tiles);
        fclose(levelFile);
        return false;
      }
      if (fileCharacter == '6') {
        if (isPlayerSpawnFound) {
          SDL_Log("Level contains multiple player spawns: %s", filePath);
          free(tiles);
          fclose(levelFile);
          return false;
        }
        isPlayerSpawnFound = true;
        spawnX = (float)(x * TILE_SIZE);
        spawnY = (float)(y * TILE_SIZE);
        // Spawn tile becomes empty ground, not a solid wall.
        tiles[y * width + x] = 0;
      } else {
        tiles[y * width + x] = (unsigned char)(fileCharacter - '0');
      }
    }
  }
  fclose(levelFile);
  if (!isPlayerSpawnFound) {
    SDL_Log("Level does not contain a player spawn: %s", filePath);
    free(tiles);
    return false;
  }
  // Commit only after fully successful load.
  level->width = width;
  level->height = height;
  level->tiles = tiles;
  level->playerSpawnX = spawnX;
  level->playerSpawnY = spawnY;
  level->hasPlayerSpawn = true;
  SDL_Log("Loaded level %s with player spawn at %.0f, %.0f", filePath, spawnX,
          spawnY);
  return true;
}
