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

typedef enum TILETYPE {
  TILE_EMPTY = 0,
  TILE_SOLID = 1,
  TILE_ONE_WAY = 2,
  TILE_BREAKABLE = 3,
  TILE_PLAYER_SPAWN = 6,
  TILE_ENEMY_SPAWN = 7
} TILETYPE;

/**
 * Loads a level from a text file into `level`. On failure, `level` is left
 * untouched (any tiles it already owned are freed first, but no partial data is
 * committed).
 * */
bool LevelLoadFromFile(Level *level, const char *filePath);
void LevelFree(Level *level);

/**
 * Encapsulated enemy access
 */
int LevelGetEnemyCount(const Level *level);

bool LevelAddEnemy(Level *level, float x, float y,
                   LevelEnemyState initialState);
int LevelGetWidth(const Level *level);
int LevelGetHeight(const Level *level);

float LevelGetWidthPixels(const Level *level);
float LevelGetHeightPixels(const Level *level);

float LevelGetPlayerSpawnX(const Level *level);
float LevelGetPlayerSpawnY(const Level *level);

unsigned char LevelGetTile(const Level *level, int tileX, int tileY);

#endif
