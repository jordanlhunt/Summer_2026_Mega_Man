#include "level.h"
void LevelGenerateTestRoom(Level *level) {
  level->width = 40;
  level->height = 24;
  level->tiles = calloc(level->width * level->height, sizeof(unsigned char));
  memset(level->tiles, 0, level->width * level->height);
  /* Floor across the entire bottom */
  for (int x = 0; x < level->width; x++) {
    level->tiles[(level->height - 2) * level->width + x] = 1;
    level->tiles[(level->height - 1) * level->width + x] = 1;
  }
  /* === ZONE 1: SPAWN / BASIC MOVEMENT (left side) === */
  /* A few small platforms to practice basic jumps */
  for (int x = 3; x < 7; x++) {
    level->tiles[(level->height - 5) * level->width + x] = 1;
  }
  for (int x = 9; x < 13; x++) {
    level->tiles[(level->height - 7) * level->width + x] = 1;
  }
  /* === ZONE 2: WALL SLIDE / WALL JUMP (center) === */
  /* Left wall of the shaft */
  for (int y = 6; y < 16; y++) {
    level->tiles[y * level->width + 18] = 1;
  }
  /* Right wall of the shaft */
  for (int y = 6; y < 16; y++) {
    level->tiles[y * level->width + 24] = 1;
  }
  /* Mid-shaft rest platform */
  for (int x = 20; x <= 22; x++) {
    level->tiles[11 * level->width + x] = 1;
  }
  /* Top exit platform */
  for (int x = 18; x <= 24; x++) {
    level->tiles[5 * level->width + x] = 1;
  }
  /* === ZONE 3: ENEMY SPAWN ARENA (right side) === */
  /* Raised platform arena for enemy testing */
  for (int x = 28; x < 38; x++) {
    level->tiles[(level->height - 4) * level->width + x] = 1;
  }
  /* A small upper platform in the arena */
  for (int x = 30; x < 36; x++) {
    level->tiles[(level->height - 8) * level->width + x] = 1;
  }
  /* Side walls */
  for (int y = 0; y < level->height; y++) {
    level->tiles[y * level->width] = 1;
    level->tiles[y * level->width + level->width - 1] = 1;
  }
}
void LevelFree(Level *level) {
  free(level->tiles);
  level->tiles = NULL;
}
void LevelRender(const Level *level, SDL_Renderer *renderer, float cameraX,
                 float cameraY) {
  int startX = (int)(cameraX / TILE_SIZE) - 1;
  int startY = (int)(cameraY / TILE_SIZE) - 1;
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
          level->tiles[(y + 1) * level->width + x] == 0 && !isFloor) {
        SDL_SetRenderDrawColor(renderer, 80, 70, 60, 255);
      } else if (isWall) {
        SDL_SetRenderDrawColor(renderer, 60, 55, 50, 255);
      } else {
        SDL_SetRenderDrawColor(renderer, 45, 40, 35, 255);
      }
      SDL_RenderFillRectF(renderer, &tile);
    }
  }
}
bool LevelLoadFromFile(Level *level, const char *filePath) {
  if (level == NULL || filePath == NULL) {
    return false;
  }
  FILE *levelFile = fopen(filePath, "r");
  if (levelFile == NULL) {
    SDL_Log("Could not open level file: %s", filePath);
    return false;
  }
  if (fscanf(levelFile, "%d %d", &levelWidth, &levelHeight) != 2) {
    SDL_Log("Could not read level dimensions from %s", filePath);
    fclose(levelFile);
    return false;
  }
  size_t tileCount = (size_t)level->width * (size_t)level->height;
  level->tiles = calloc(tileCount, sizeof(*level->tiles));
  if (level->tiles == NULL) {
    SDL_Log("Could not allocate memory for level: %s", filePath);
    level->width = 0;
    level->height = 0;
    fclose(levelFile);
    return false;
  }
  bool isPlayerSpawnFound = false;
  float spawnX = 0.0f;
  float spawnY = 0.0f;
  char fileCharacter; // Changed to char to match fscanf's %c expectation
  for (int y = 0; y < levelHeight; y++) {
    for (int x = 0; x < levelWidth; x++) {
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
        tiles[y * levelWidth + x] = 0;
      } else {
        tiles[y * levelWidth + x] = (unsigned char)(fileCharacter - '0');
      }
    }
  }
  fclose(levelFile);
  if (!isPlayerSpawnFound) {
    SDL_Log("Level does not contain a player spawn: %s", filePath);
    free(tiles);
    return false;
  }
  level->width = levelWidth;
  level->height = levelHeight;
  level->tiles = tiles;
  level->playerSpawnX = spawnX;
  level->playerSpawnY = spawnY;
  level->hasPlayerSpawn = true;
  SDL_Log("Loaded level %s with player spawn at %.0f, %.0f", filePath, spawnX,
          spawnY);
  return true;
}