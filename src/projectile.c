#include "projectile.h"

bool ProjectileSpawn(Projectile projectiles[MAX_PROJECTILES], float originX,
                     float originY, bool isFacingRight) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!projectiles[i].isActive) {
      float velocityX;
      if (isFacingRight) {
        velocityX = PROJECTILE_SPEED;
      } else {
        velocityX = -PROJECTILE_SPEED;
      }
      projectiles[i] = (Projectile){
          .x = originX,
          .y = originY,
          .velocityX = velocityX,
          .lifeTimer = PROJECTILE_LIFETIME,
          .isActive = true,
      };
      return true;
    }
  }
  return false;
}
void ProjectileUpdateAll(Projectile projectiles[MAX_PROJECTILES],
                         const Level *level, float deltaTime) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    Projectile *projectile = &projectiles[i];
    if (!projectile->isActive) {
      continue;
    }
    projectile->lifeTimer -= deltaTime;
    if (projectile->lifeTimer <= 0.0f) {
      projectile->isActive = false;
      continue;
    }
    // Calculate movement for this frame
    float totalMoveX = projectile->velocityX * deltaTime;
    // Sub-step movement to prevent passing through thin walls (tunneling)
    float stepSize = PROJECTILE_SUBSTEP_SIZE;
    int steps = (int)ceilf(fabsf(totalMoveX) / stepSize);
    if (steps < 1) {
      steps = 1;
    }
    float stepX = totalMoveX / steps;
    for (int step = 0; step < steps; step++) {
      projectile->x += stepX;
      // Full AABB check against solid tiles at current sub-step
      if (CollisionCheckAABB(level, projectile->x, projectile->y,
                             PROJECTILE_WIDTH, PROJECTILE_HEIGHT)) {
        projectile->isActive = false;
        break;
      }
    }
  }
}
void ProjectileRenderAll(const Projectile projectiles[MAX_PROJECTILES],
                         SDL_Renderer *renderer, float cameraX, float cameraY,
                         SDL_Texture *projectileTexture) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    const Projectile *projectile = &projectiles[i];
    if (!projectile->isActive) {
      continue;
    }
    SDL_FRect destinationRect = {.x = projectile->x - cameraX,
                                 .y = projectile->y - cameraY,
                                 .w = PROJECTILE_WIDTH,
                                 .h = PROJECTILE_HEIGHT};
    SDL_RenderTexture(renderer, projectileTexture, NULL, &destinationRect);
  }
}
void ProjectileHandlePlayerShooting(Projectile projectiles[MAX_PROJECTILES],
                                    Player *player, const Input *input) {
  if (input->isShootJustPressed && player->shootCooldown <= 0.0f) {
    float originY = player->y + player->height * 0.5f -
                    PROJECTILE_HEIGHT * 0.5f - PROJECTILE_MUZZLE_OFFSET_Y;
    float originX;
    if (player->isFacingRight) {
      originX = player->x + player->width;
    } else {
      originX = player->x - PROJECTILE_WIDTH;
    }
    if (ProjectileSpawn(projectiles, originX, originY, player->isFacingRight)) {
      player->shootCooldown = PROJECTILE_COOLDOWN;
      player->shootAnimationTimer = SHOOT_ANIMATION_DURATION;
    }
  }
}
