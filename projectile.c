#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "config.h"
#include "level.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#define MAX_PROJECTILES 16
#define PROJECTILE_WIDTH 11.0f
#define PROJECTILE_HEIGHT 11.0f
#define PROJECTILE_SPEED 420.0f
#define PROJECTILE_LIFETIME 1.2f
#define PROJECTILE_COOLDOWN 0.22f

typedef struct Projectile {
  float x;
  float y;
  float velocityX;
  float lifeTimer;
  bool isActive;

} Projectile;

#endif
