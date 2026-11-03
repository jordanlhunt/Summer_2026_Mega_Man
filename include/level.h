#ifndef LEVEL_H
#define LEVEL_H
#include "common.h"
#include "config.h"
#include "healthpickup.h"
#include "levelEnemy.h"

#define LEVEL_FLOOR_ROW 21
#define MAX_LEVEL_ENEMIES 7
#define MAX_LEVEL_PATH_LENGTH 128
#define TILE_BREAKABLE_CHAR '3'
#define TILE_EMPTY_CHAR '0'
#define TILE_ENEMY_SPAWN_CHAR '7'
#define TILE_ONE_WAY_CHAR '2'
#define TILE_PLAYER_SPAWN_CHAR '6'
#define TILE_SOLID_CHAR '1'
#define TILE_SPIKE_CHAR 'S'
#define TILE_HEALTH_CHAR 'H'
#define TRANSITION_TYPE_EDGE "edge"

typedef struct Level {
  bool hasPlayerSpawn;

  float playerSpawnX;
  float playerSpawnY;
  int height;
  int levelEnemiesCount;
  int width;
  LevelEnemy levelEnemies[MAX_LEVEL_ENEMIES];
  unsigned char *tiles;
  char edgeLeft[MAX_LEVEL_PATH_LENGTH];
  char edgeRight[MAX_LEVEL_PATH_LENGTH];
  char edgeUp[MAX_LEVEL_PATH_LENGTH];
  char edgeDown[MAX_LEVEL_PATH_LENGTH];
  HealthPickup healthPickups[MAX_HEALTH_PICKUPS];
  int healthPickupCount;
} Level;
typedef enum TILETYPE {
  TILE_EMPTY = 0,
  TILE_SOLID = 1,
  TILE_ONE_WAY = 2,
  TILE_BREAKABLE = 3,
  TILE_SPIKE = 4,
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
bool LevelAddHealthPickup(Level *level, float x, float y);
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
