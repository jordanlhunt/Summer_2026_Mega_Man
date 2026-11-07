#include "collision.h"
#include "config.h"
#include "graphics.h"
#include "healthpickup.h"
#include "level.h"
#include "player.h"
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

// Private Helpers
static void HealthPickupRenderFallback(SDL_Renderer *renderer,
                                       SDL_FRect destination) {
  SDL_SetRenderDrawColor(renderer, 40, 220, 90, 255);
  SDL_RenderFillRect(renderer, &destination);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_FRect vertical = {.x = destination.x + destination.w * 0.35f,
                        .y = destination.y + destination.h * 0.10f,
                        .w = destination.w * 0.30f,
                        .h = destination.h * 0.80f};
  SDL_FRect horizontal = {.x = destination.x + destination.w * 0.10f,
                          .y = destination.y + destination.h * 0.35f,
                          .w = destination.w * 0.80f,
                          .h = destination.h * 0.30f};

  SDL_RenderFillRect(renderer, &vertical);
  SDL_RenderFillRect(renderer, &horizontal);
}

void HealthPickUpInitialize(HealthPickup *healthPickup, float x, float y) {
  *healthPickup = (HealthPickup){
      .entity = {.x = x,
                 .y = y,
                 .width = HEALTH_PICKUP_SIZE,
                 .height = HEALTH_PICKUP_SIZE,
                 .velocityX = 0.0f,
                 .velocityY = 0.0f,
                 .isActive = true,
                 .isOnGround = false},
      .sinCurveBobTimer = 0.0f,
  };
}

void HealthPickupUpdate(HealthPickup *healthPickup, const Level *level,
                        float deltaTime) {
  if (!healthPickup->entity.isActive) {
    return;
  }
  healthPickup->sinCurveBobTimer += deltaTime;
  // Give picks a semi random trajectory to fall toward the player's direction
  healthPickup->entity.velocityY += GRAVITY * deltaTime;
  if (healthPickup->entity.velocityY > MAX_FALL_SPEED) {
    healthPickup->entity.velocityY = MAX_FALL_SPEED;
  }
  EntityMove(&healthPickup->entity, deltaTime);
  CollisionResolveTileAxis(&healthPickup->entity, level, false);
  // If the healthPickup falls of the map delete it
  if (healthPickup->entity.y > LevelGetHeightPixels(level)) {
    healthPickup->entity.isActive = false;
  }
}
void HealthPickupUpdateAll(HealthPickup healthPickups[], int count,
                           const Level *level, float deltaTime) {
  for (int i = 0; i < count; i++) {
    HealthPickupUpdate(&healthPickups[i], level, deltaTime);
  }
}

void HealthPickupCollectAll(HealthPickup healthPickups[], int count,
                            Player *player) {
  if (player->hitPoints <= 0) {
    return;
  }
  for (int i = 0; i < count; i++) {
    HealthPickup *healthPickup = &healthPickups[i];
    if (!healthPickup->entity.isActive) {
      continue;
    }
    if (!EntityOverlaps(&healthPickup->entity, &player->entity)) {
      continue;
    }
    // If player is already at MAX_HITPOINTS leave healthPickup on the ground
    if (player->hitPoints >= MAX_HITPOINTS) {
      continue;
    }
    player->hitPoints += HEALTH_PICKUP_HP_RESTORE;
    if (player->hitPoints > MAX_HITPOINTS) {
      player->hitPoints = MAX_HITPOINTS;
    }
    healthPickup->entity.isActive = false;
  }
}

void HealthPickupRenderAll(const HealthPickup healthPickups[], int count,
                           SDL_Renderer *sdlRenderer, const Camera *camera,
                           SDL_Texture *healthPickupTexture) {
  for (int i = 0; i < count; i++) {
    const HealthPickup *healthPickup = &healthPickups[i];
    if (!healthPickup->entity.isActive) {
      continue;
    }
    float sinCurveBob =
        sinf(healthPickup->sinCurveBobTimer * HEALTH_PICKUP_BOB_SPEED) *
        HEALTH_PICKUP_BOB_HEIGHT;
    SDL_FRect destinationRect = {.x = healthPickup->entity.x - camera->x,
                                 .y = healthPickup->entity.y - camera->y +
                                      sinCurveBob,
                                 .w = healthPickup->entity.width,
                                 .h = healthPickup->entity.height};
    if (healthPickupTexture == NULL) {
      HealthPickupRenderFallback(sdlRenderer, destinationRect);
      continue;
    }
    SDL_FRect sourceRect = {.x = HEALTH_PICKUP_SIZE * HEALTH_PICKUP_FRAME_SIZE,
                            .y = HEALTH_PICKUP_SIZE * HEALTH_PICKUP_FRAME_SIZE,
                            .w = HEALTH_PICKUP_SIZE * HEALTH_PICKUP_FRAME_SIZE,
                            .h = HEALTH_PICKUP_FRAME_SIZE};
    SDL_RenderTexture(sdlRenderer, healthPickupTexture, &sourceRect,
                      &destinationRect);
  }
}
