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

void ProjectileUpdateAll(Projectile *projectiles, const Level *level,
                         float deltaTime) {
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
void ProjectileRenderAll(const Projectile *projectiles, SDL_Renderer *renderer,
                         float cameraX, float cameraY,
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
    float originY =
        player->y + player->height * 0.5f - PROJECTILE_HEIGHT * 0.5f;
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
