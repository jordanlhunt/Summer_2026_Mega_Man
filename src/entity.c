#include "entity.h"

bool EntityOverlaps(const Entity *a, const Entity *b) {
  if (!a || !b) {
    return false;
  }
  return (a->x < b->x + b->width) && (a->x + a->width > b->x) &&
         (a->y < b->y + b->height) && (a->y + a->height > b->y);
}
void EntityInitialize(Entity *entity, float x, float y, float width,
                      float height) {
  entity->x = x;
  entity->y = y;
  entity->width = width;
  entity->height = height;
  entity->velocityX = 0.0f;
  entity->velocityY = 0.0f;
  entity->isActive = true;
}

void EntityMove(Entity *entity, float deltaTime) {
  entity->x += entity->velocityX * deltaTime;
  entity->y += entity->velocityY * deltaTime;
}
