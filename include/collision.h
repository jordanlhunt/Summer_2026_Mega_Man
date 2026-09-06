#ifndef COLLISION_H
#define COLLISION_H
#include "common.h"
#include "config.h"
#include "entity.h"
#include "forwarddeclares.h"

/**
 * Returns true if two AABBs overlap
 */
bool CollisionAABBBoxOverlap(float x1, float y1, float width1, float height1,
                             float x2, float y2, float width2, float height2);

bool CollisionIsSolidTile(const Level *level, int tileX, int tileY);

/**
 * Checks if a line of tiles along a single row or column contains any solid
 * tile. If isVerticalLine is true, edgeTile is the X coordinate,
 * startTile/endTile are Y coordinates. If false, edgeTile is the Y coordinate,
 * startTile/endTile are X coordinates.
 */
bool CollisionCheckTileEdge(const Level *level, int edgeTile, int startTile,
                            int endTile, bool isVerticalLine);

/**
 * Returns true if the AABB overlaps any solid tile.
 */
bool CollisionCheckAABB(const Level *level, float x, float y, float width,
                        float height);

/**
 * Resolves the AABB along one axis. Repeatedly pushes the box out of any
 * overlapping solid tiles until none remain. Sets isOnGround if landing.
 */
void CollisionResolveTileAxis(Entity *entity, const Level *level, bool isXAxis);

/* Wall‑slide queries (refactored to use CollisionCheckTileEdge) */
bool CollisionCheckWallLeft(const Level *level, float x, float y, float height);
bool CollisionCheckWallRight(const Level *level, float x, float y, float width,
                             float height);

/**
 * Scans the tiles overlapping the given AAB and sets any tile matching
 'tileType' to 0 (empty). This level is modified in place
 */
void CollisionRemoveTilesOfType(Level *level, float x, float y, float width,
                                float height, unsigned char tileType);

#endif
