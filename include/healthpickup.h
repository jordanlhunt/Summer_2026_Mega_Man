#ifndef HEALTHPICKUP_H
#define HEALTHPICKUP_H

#include "common.h"
#include "entity.h"
#include "forwarddeclares.h"

#define MAX_HEALTH_PICKUPS 16
#define HEALTH_PICKUP_SIZE 16
#define HEALTH_PICKUP_HP_RESTORE 4

typedef struct HealthPickup {
  Entity entity;
  float sinCurveBobTimer;
  bool isActive;
} HealthPickup;

void HealthPickupInitialize(HealthPickup *healthPickUp, float x, float y);
void HealthPickupUpdate(HealthPickup *healthPickUp, const Level *level,
                        float deltaTime);
void HealthPickupCollectAll(HealthPickup pickups[], int count, Player *player);
void HealthPickupRenderAll(const HealthPickup healthPickups[], int count,
                           SDL_Renderer *sdlRenderer, const Camera *camera);

#endif