#ifndef HEALTHPICKUP_H
#define HEALTHPICKUP_H

#include "common.h"
#include "entity.h"
#include "forwarddeclares.h"

#define MAX_HEALTH_PICKUPS 16
#define HEALTH_PICKUP_SIZE 16.0f
#define HEALTH_PICKUP_HP_RESTORE 4
#define HEALTH_DROP_CHANCE 0.35f
#define HEALTH_PICKUP_BOB_SPEED 4.0f
#define HEALTH_PICKUP_BOB_HEIGHT 3.0f
#define HEALTH_PICKUP_FRAME_SIZE 16
#define HEALTH_PICKUP_FRAME_X 0
#define HEALTH_PICKUP_FRAME_Y 0

typedef struct HealthPickup {
  Entity entity;
  float sinCurveBobTimer;
} HealthPickup;

void HealthPickupInitialize(HealthPickup *healthPickUp, float x, float y);
void HealthPickupUpdate(HealthPickup *healthPickUp, const Level *level,
                        float deltaTime);
void HealthPickupUpdateAll(HealthPickup healthPickups[], int count,
                           const Level *level, float deltaTime);
void HealthPickupCollectAll(HealthPickup pickups[], int count, Player *player);
void HealthPickupRenderAll(const HealthPickup healthPickups[], int count,
                           SDL_Renderer *sdlRenderer, const Camera *camera,
                           SDL_Texture *healthPickupTexture);

#endif