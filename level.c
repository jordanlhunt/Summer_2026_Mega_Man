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