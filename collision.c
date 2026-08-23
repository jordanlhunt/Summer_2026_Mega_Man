#include "collision.h"
#include "game.h"

bool CollisionIsSolidTile(const Level *level, int tileX, int tileY) {
  if (tileX < 0 || tileX >= level->width || tileY < 0 ||
      tileY >= level->height) {
    return false;
  }
  return level->tiles[tileY * level->width + tileX] != 0;
}
void CollisionResolveTileAxis(AxisAlignedBoundingBox *boundingBox,
                              const Level *level, bool *isXAxis) {
  int startTileX = (int)floorf(boundingBox->x / TILE_SIZE);
  int startTileY = (int)floorf(boundingBox->y / TILE_SIZE);
  int endTileX =
      (int)floorf((boundingBox->x + boundingBox->width - 0.01f) / TILE_SIZE);
  int endTileY =
      (int)floorf((boundingBox->y + boundingBox->height - 0.01f) / TILE_SIZE);
  for (int tileY = startTileY; tileY <= endTileY; tileY++) {
    for (int tileX = startTileX; tileX <= endTileX; tileX++) {
      if (!CollisionIsSolidTile(level, tileX, tileY)) {
        continue;
      }
      float tileXPosition = (float)(tileX * TILE_SIZE);
      float tileYPosition = (float)(tileY * TILE_SIZE);
      if (isXAxis) {
        if (boundingBox->velocityX > 0.0f) {
          boundingBox->x = tileXPosition - boundingBox->width - 0.01f;
        } else if (boundingBox->velocityX < 0.0f) {
          boundingBox->x = tileXPosition + TILE_SIZE + 0.01f;
        }
        boundingBox->velocityX = 0.0f;
      } else {
        // Landing on the floor
        if (boundingBox->velocityY > 0.0f) {
          boundingBox->y = tileYPosition - boundingBox->height - 0.01f;
          boundingBox->isOnGround = true;
        }
        // Hitting the ceiling
        else if (boundingBox->velocityY < 0.0f) {
          boundingBox->y = tileYPosition + TILE_SIZE + 0.01f;
        }
        boundingBox->velocityY = 0.0f;
      }
    }
  }
}

bool CollisionCheckWallLeft(const Level *level, float x, float y,
                            float height) {
  int tileX = (int)floorf((x - 1.0f) / TILE_SIZE);
  int tileYTop = (int)floorf((y + 2.0f) / TILE_SIZE);
  int tileYBottom = (int)floorf((y + height - 2.0f) / TILE_SIZE);

  for (int tileY = tileYTop; tileY <= tileYBottom; tileY++) {
    if (CollisionIsSolidTile(level, tileX, tileY)) {
      return true;
    }
  }
  return false;
}
bool CollisionCheckWallRight(const Level *level, float x, float y, float width,
                             float height) {
  int tileX = (int)floorf((x + width + 1.0f) / TILE_SIZE);
  int tileYTop = (int)floorf((y + 2.0f) / TILE_SIZE);
  int tileYBottom = (int)floorf((y + height - 2.0f) / TILE_SIZE);
  for (int tileY = tileYTop; tileY <= tileYBottom; tileY++) {
    if (CollisionIsSolidTile(level, tileX, tileY)) {
      return true;
    }
  }
  return false;
}
