#include "projectile.h"

void ProjectileSpawn(Projectile projectiles[MAX_PROJECTILES], float originX,
                     float originY, bool isFacingRight) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!projectiles[i].isActive) {
      projectiles[i].isActive = true;
      projectiles[i].x = originX;
      projectiles[i].y = originY;
      if (isFacingRight) {
        projectiles[i].velocityX = PROJECTILE_SPEED;
      } else {
        projectiles[i].velocityX = -PROJECTILE_SPEED;
      }
      projectiles[i].lifeTimer = PROJECTILE_LIFETIME;
      return;
    }
  }
  // Pool exhausted - shot is dropped
}

void ProjectileUpdateAll(Projectile *projectiles, const Level *level,
                         float deltaTime) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    Projectile *projectile = &projectiles[i];
    if (!projectile->isActive) {
      continue;
    }
    projectile->x += projectile->velocityX * deltaTime;
    int tileX =
        (int)floorf((projectile->x + PROJECTILE_WIDTH * 0.5f) / TILE_SIZE);
    int tileY =
        (int)floorf((projectile->y + PROJECTILE_HEIGHT * 0.5f) / TILE_SIZE);
    if (CollisionIsSolidTile(level, tileX, tileY)) {
      projectile->isActive = false;
    }
  }
}
void ProjectileRenderAll(const Projectile *projectiles, int *renderer,
                         float cameraX, float cameraY, int *projectileTexture) {
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
    float originY = player->y + player->height * .5f - PROJECTILE_HEIGHT * .5f;
    float originX;
    if (player->isFacingRight) {
      originX = player->x + player->width;
    } else {
      originX = player->x - PROJECTILE_WIDTH;
    }
    ProjectileSpawn(projectiles, originX, originY, player->isFacingRight);
    player->shootCooldown = PROJECTILE_COOLDOWN;
  }
}
