#ifndef LEVEL_H
#define LEVEL_H
#include "common.h"
#include "config.h"
#include "levelEnemy.h"
#define MAX_LEVEL_ENEMIES 7
typedef struct Level {
  int width;
  int height;
  unsigned char *tiles;
  float playerSpawnX;
  float playerSpawnY;
  bool hasPlayerSpawn;
  LevelEnemy levelEnemies[MAX_LEVEL_ENEMIES];
  int levelEnemiesCount;
} Level;
/**
 * Loads a level from a text file into `level`. On failure, `level` is left
 * untouched (any tiles it already owned are freed first, but no partial data is
 * committed).
 * */
bool LevelLoadFromFile(Level *level, const char *filePath);
void LevelFree(Level *level);
#endif
