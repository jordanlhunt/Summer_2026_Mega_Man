#include "projectile.h"
#include "collision.h"
#include "entity.h"
#include "level.h"
#include "levelEnemy.h"
#include <stdbool.h>

bool ProjectileSpawn(Projectile projectiles[MAX_PROJECTILES], float originX,
                     float originY, bool isFacingRight) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!projectiles[i].entity.isActive) {
      float velocityX;
      if (isFacingRight) {
        velocityX = PROJECTILE_SPEED;
      } else {
        velocityX = -PROJECTILE_SPEED;
      }
      projectiles[i] = (Projectile){
          .entity =
              {
                  .x = originX,
                  .y = originY,
                  .width = PROJECTILE_WIDTH,
                  .height = PROJECTILE_HEIGHT,
                  .velocityX = velocityX,
                  .velocityY = 0.0f,
                  .isActive = true,
              },
          .lifeTimer = PROJECTILE_LIFETIME,
      };
      return true;
    }
  }
  return false;
}

void ProjectileUpdateAll(Projectile projectiles[MAX_PROJECTILES], Level *level,
                         float deltaTime) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    Projectile *projectile = &projectiles[i];
    if (!projectile->entity.isActive) {
      continue;
    }
    projectile->lifeTimer -= deltaTime;
    if (projectile->lifeTimer <= 0.0f) {
      projectile->entity.isActive = false;
      continue;
    }
    float totalMoveX = projectile->entity.velocityX * deltaTime;
    float stepSize = PROJECTILE_SUBSTEP_SIZE;
    int steps = (int)ceilf(fabsf(totalMoveX) / stepSize);
    if (steps < 1) {
      steps = 1;
    }
    float stepX = totalMoveX / steps;
    for (int step = 0; step < steps; step++) {
      projectile->entity.x += stepX;
      if (CollisionCheckAABB(level, projectile->entity.x, projectile->entity.y,
                             PROJECTILE_WIDTH, PROJECTILE_HEIGHT)) {
        // Remove any breakable tiles (type 3) at the impact position
        CollisionRemoveTilesOfType(level, projectile->entity.x,
                                   projectile->entity.y, PROJECTILE_WIDTH,
                                   PROJECTILE_HEIGHT, TILE_BREAKABLE);
        projectile->entity.isActive = false;
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
    if (!projectile->entity.isActive) {
      continue;
    }
    SDL_FRect destinationRect = {.x = projectile->entity.x - cameraX,
                                 .y = projectile->entity.y - cameraY,
                                 .w = projectile->entity.width,
                                 .h = projectile->entity.height};
    SDL_RenderTexture(renderer, projectileTexture, NULL, &destinationRect);
  }
}

void ProjectileHandlePlayerShooting(Projectile projectiles[MAX_PROJECTILES],
                                    Player *player, const Input *input) {
  if (input->isShootJustPressed && player->shootCooldown <= 0.0f) {
    float originY = player->entity.y + player->entity.height * 0.5f -
                    PROJECTILE_HEIGHT * 0.5f - PROJECTILE_MUZZLE_OFFSET_Y;
    float originX;
    if (player->isFacingRight) {
      originX = player->entity.x + player->entity.width;
    } else {
      originX = player->entity.x - PROJECTILE_WIDTH;
    }
    if (ProjectileSpawn(projectiles, originX, originY, player->isFacingRight)) {
      player->shootCooldown = PROJECTILE_COOLDOWN;
      player->shootAnimationTimer = SHOOT_ANIMATION_DURATION;
    }
  }
}

void HandleProjectileEntityCollision(

    Projectile projectiles[MAX_PROJECTILES],

    LevelEnemy enemies[],

    int enemyCount) {
  for (int projectileIndex = 0; projectileIndex < MAX_PROJECTILES;
       projectileIndex++) {
    Projectile *projectile = &projectiles[projectileIndex];
    if (!projectile->entity.isActive) {
      continue;
    }
    for (int enemyIndex = 0; enemyIndex < enemyCount; enemyIndex++) {
      LevelEnemy *levelEnemy = &enemies[enemyIndex];
      if (!levelEnemy->entity.isActive) {
        continue;
      }
      if (!EntityOverlaps(&projectile->entity, &levelEnemy->entity)) {
        continue;
      }
      projectile->entity.isActive = false;
      LevelEnemyApplyDamage(levelEnemy, 1);
      break;
    }
  }
}
