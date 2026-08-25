#ifndef LEVEL_H
#define LEVEL_H
#include "config.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct Level {
  int width;
  int height;
  unsigned char *tiles;
  float playerSpawnX;
  float playerSpawnY;
  bool hasPlayerSpawn;
} Level;
/**
 * Loads a level from a text file into `level`. On failure, `level` is left
 * untouched (any tiles it already owned are freed first, but no partial data is
 * committed).
 * */
bool LevelLoadFromFile(Level *level, const char *filePath);
void LevelFree(Level *level);
#endif
