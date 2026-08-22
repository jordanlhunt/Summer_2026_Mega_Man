#ifndef COLLISION_H
#define COLLISION_H
#include "config.h"
#include "game.h"
#include "level.h"
#include <math.h>
#include <stdbool.h>

/**
 * A minimal axis-aligned box used for tile collision. Any entity can build one
 * from their own fields and send it to CollisionResolveTileAxis, and copy the
 * result back.
 */
typedef struct AxisAlignedBoundingBox {
  float x;
  float y;
  float width;
  float height;
  float velocityX;
  float velocityY;
  bool isOnGround;
} AxisAlignedBoundingBox;

/**
 * True if the tile at (tileX, tileY) is solid. Out-of-bounds is never solid
 */
bool CollisionIsSolidTile(const Level *level, int tileX, int tileY);
/**
 * Resolves collision along a single axis. Mutates box->x/y and zeroes the
 * zeroes the relevant velocity component on impact; sets box->isOnGround when
 * landing on top of a tile during Y-axis pass
 */
void CollisionResolveTileAxis(AxisAlignedBoundingBox *boundingBox,
                              const Level *level, bool *isXAxis);
/* Tile queries used for wall-slide/wall-jump detection. */
bool CollisionCheckWallLeft(const Level *level, float x, float y, float height);
bool CollisionCheckWallRight(const Level *level, float x, float y, float width,
                             float height);

#endif
