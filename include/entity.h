#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"

/**
 * Base entity structure containing transform, physics, and lifetime state.
 */
typedef struct Entity {
  float x;
  float y;
  float width;
  float height;
  float velocityX;
  float velocityY;
  bool isActive;
  bool isOnGround;
} Entity;

/**
 *  AABB overlap using Entity
 */
bool EntityOverlaps(const Entity *a, const Entity *b);

/**
 * Move entity by its velocity, with optional deltaTime scaling.
 */
void EntityMove(Entity *entity, float deltaTime);

#endif
