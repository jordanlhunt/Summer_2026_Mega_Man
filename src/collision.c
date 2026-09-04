#include "collision.h"
#include "level.h"

// ------------------------------------------------------------
// Basic tile collision
// ------------------------------------------------------------
bool CollisionIsSolidTile(const Level *level, int tileX, int tileY) {
  unsigned char tile = LevelGetTile(level, tileX, tileY);
  return (tile == 1 || tile == 3);
}
// ------------------------------------------------------------
// Remove a tile upon collision
// ------------------------------------------------------------
void CollisionRemoveTilesOfType(Level *level, float x, float y, float width,
                                float height, unsigned char tileType) {
  // Scan overlapping tiles for breaking blocks
  int startTileX = (int)floorf(x / TILE_SIZE);
  int endTileX = (int)floorf((x + width - 0.01f) / TILE_SIZE);
  int startTileY = (int)floorf(y / TILE_SIZE);
  int endTileY = (int)floorf((y + height - 0.01f) / TILE_SIZE);
  for (int tileY = startTileY; tileY <= endTileY; tileY++) {
    for (int tileX = startTileX; tileX <= endTileX; tileX++) {
      if (tileX < 0 || tileX >= level->width || tileY < 0 ||
          tileY >= level->height) {
        continue;
      }
      size_t index = (size_t)tileY * level->width + (size_t)tileX;
      if (level->tiles[index] == tileType) {
        level->tiles[index] = 0;
      }
    }
  }
}
// ------------------------------------------------------------
// Edge / line check
// ------------------------------------------------------------
bool CollisionCheckTileEdge(const Level *level, int edgeTile, int startTile,
                            int endTile, bool isVerticalLine) {
  for (int i = startTile; i <= endTile; i++) {
    int tileX, tileY;
    if (isVerticalLine) {
      tileX = edgeTile;
      tileY = i;
    } else {
      tileX = i;
      tileY = edgeTile;
    }
    if (CollisionIsSolidTile(level, tileX, tileY)) {
      return true;
    }
  }
  return false;
}
// ------------------------------------------------------------
// AABB overlap test
// ------------------------------------------------------------
bool CollisionCheckAABB(const Level *level, float x, float y, float width,
                        float height) {
  int startTileX = (int)floorf(x / TILE_SIZE);
  int endTileX = (int)floorf((x + width - 0.01f) / TILE_SIZE);
  int startTileY = (int)floorf(y / TILE_SIZE);
  int endTileY = (int)floorf((y + height - 0.01f) / TILE_SIZE);
  for (int tileY = startTileY; tileY <= endTileY; tileY++) {
    for (int tileX = startTileX; tileX <= endTileX; tileX++) {
      if (CollisionIsSolidTile(level, tileX, tileY)) {
        return true;
      }
    }
  }
  return false;
}
// ------------------------------------------------------------
// Helper: returns the first overlapping tile
// ------------------------------------------------------------
static bool FindFirstOverlappingTile(const Level *level, float x, float y,
                                     float width, float height, int *outTileX,
                                     int *outTileY) {
  int startTileX = (int)floorf(x / TILE_SIZE);
  int endTileX = (int)floorf((x + width - 0.01f) / TILE_SIZE);
  int startTileY = (int)floorf(y / TILE_SIZE);
  int endTileY = (int)floorf((y + height - 0.01f) / TILE_SIZE);
  for (int tileY = startTileY; tileY <= endTileY; tileY++) {
    for (int tileX = startTileX; tileX <= endTileX; tileX++) {
      if (CollisionIsSolidTile(level, tileX, tileY)) {
        if (outTileX)
          *outTileX = tileX;
        if (outTileY)
          *outTileY = tileY;
        return true;
      }
    }
  }
  return false;
}
// ------------------------------------------------------------
// Iterative resolution along one axis
// ------------------------------------------------------------
void CollisionResolveTileAxis(AxisAlignedBoundingBox *boundingBox,
                              const Level *level, bool isXAxis) {
  const int MAX_ITERATIONS = 8;
  for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
    int tileX, tileY;
    if (!FindFirstOverlappingTile(level, boundingBox->x, boundingBox->y,
                                  boundingBox->width, boundingBox->height,
                                  &tileX, &tileY)) {
      break;
    }
    float tileXPosition = (float)(tileX * TILE_SIZE);
    float tileYPosition = (float)(tileY * TILE_SIZE);
    // 3. Resolve along the axis, push the box out.
    if (isXAxis) {
      if (boundingBox->velocityX > 0.0f) {
        boundingBox->x = tileXPosition - boundingBox->width - 0.01f;
      } else if (boundingBox->velocityX < 0.0f) {
        boundingBox->x = tileXPosition + TILE_SIZE + 0.01f;
      }
      boundingBox->velocityX = 0.0f;
    } else {
      if (boundingBox->velocityY > 0.0f) {
        boundingBox->y = tileYPosition - boundingBox->height - 0.01f;
        boundingBox->isOnGround = true;
      } else if (boundingBox->velocityY < 0.0f) {
        boundingBox->y = tileYPosition + TILE_SIZE + 0.01f;
      }
      boundingBox->velocityY = 0.0f;
    }
  }
}
// ------------------------------------------------------------
// Wall‑side checks
// ------------------------------------------------------------
bool CollisionCheckWallLeft(const Level *level, float x, float y,
                            float height) {
  int tileX = (int)floorf((x - 1.0f) / TILE_SIZE);
  int tileYTop = (int)floorf((y + 2.0f) / TILE_SIZE);
  int tileYBottom = (int)floorf((y + height - 2.0f) / TILE_SIZE);
  return CollisionCheckTileEdge(level, tileX, tileYTop, tileYBottom, true);
}
bool CollisionCheckWallRight(const Level *level, float x, float y, float width,
                             float height) {
  int tileX = (int)floorf((x + width + 1.0f) / TILE_SIZE);
  int tileYTop = (int)floorf((y + 2.0f) / TILE_SIZE);
  int tileYBottom = (int)floorf((y + height - 2.0f) / TILE_SIZE);
  return CollisionCheckTileEdge(level, tileX, tileYTop, tileYBottom, true);
}

bool CollisionAABBBoxOverlap(float x1, float y1, float w1, float h1, float x2,
                             float y2, float w2, float h2) {
  return (x1 < x2 + w2) && (x1 + w1 > x2) && (y1 < y2 + h2) && (y1 + h1 > y2);
}
