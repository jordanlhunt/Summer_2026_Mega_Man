#include "game.h"
#include "config.h"
#include "graphics.h"
#include "level.h"
#include "levelEnemy.h"
#include "projectile.h"
#include <SDL3/SDL_render.h>
bool GameInitialize(Game *game) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return false;
  }
  game->gameWindow = SDL_CreateWindow("SDL3 - Robot Hero Moby", SCREEN_WIDTH,
                                      SCREEN_HEIGHT, 0);
  if (game->gameWindow == NULL) {
    SDL_Log("Unable to create window: %s", SDL_GetError());
    SDL_Quit();
    return false;
  }
  game->gameRenderer = SDL_CreateRenderer(game->gameWindow, NULL);
  if (game->gameRenderer == NULL) {
    SDL_Log("Unable to create renderer: %s", SDL_GetError());
    SDL_DestroyWindow(game->gameWindow);
    game->gameWindow = NULL;
    SDL_Quit();
    return false;
  }
  if (!SDL_SetRenderVSync(game->gameRenderer, 1)) {
    SDL_Log("Warning: VSync not enabled: %s", SDL_GetError());
  }
  // Load sprite sheet
  game->spriteSheetTexture =
      GraphicsLoadSpriteSheet(game->gameRenderer, SARABOT_ASSET_PATH);
  if (game->spriteSheetTexture == NULL) {
    SDL_Log("Failed to load sprite sheet.");
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    SDL_Quit();
    return false;
  }
  // Load projectile sprite
  game->playerProjectileTexture =
      GraphicsLoadSpriteSheet(game->gameRenderer, PLAYER_PROJECTILE_ASSET_PATH);
  if (game->playerProjectileTexture == NULL) {
    SDL_Log("Failed to load projectile sprite.");
    SDL_DestroyTexture(game->spriteSheetTexture);
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    SDL_Quit();
    return false;
  }

  // Load the level enemy texture
  game->levelEnemyTexture =
      GraphicsLoadSpriteSheet(game->gameRenderer, BIG_PROPELLER_BOT_ASSET_PATH);
  if (game->levelEnemyTexture == NULL) {
    SDL_Log("Failed to load Big Propeller sprite.");
    SDL_DestroyTexture(game->spriteSheetTexture);
    SDL_DestroyTexture(game->playerProjectileTexture);
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    SDL_Quit();
    return false;
  }
  game->player = (Player){.entity =
                              {
                                  .x = 0,
                                  .y = 0,
                                  .width = 32,
                                  .height = 40,
                                  .velocityX = 0,
                                  .velocityY = 0,
                                  .isActive = true,
                              },
                          .isFacingRight = true,
                          .hitPoints = 16,
                          .styleRow = STYLE_NES,
                          .currentPlayerState = STATE_IDLE};
  if (!LevelLoadFromFile(&game->level, "assets/levels/testroom.txt")) {
    SDL_Log("Failed to load level file. %s", SDL_GetError());
    // Clean up anything created so far:
    SDL_DestroyTexture(game->spriteSheetTexture);
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    SDL_Quit();
    return false;
  }
  game->camera.x = 0;
  game->camera.y = 0;
  game->player.entity.x = game->level.playerSpawnX;
  game->player.entity.y = game->level.playerSpawnY;
  game->isRunning = true;
  game->previousTime = SDL_GetTicks();

  memset(game->projectiles, 0, sizeof(game->projectiles));
  InputInitialize(&game->input);
  return true;
}

void GameShutdown(Game *game) {
  if (game == NULL) {
    return;
  }
  SDL_DestroyTexture(game->levelEnemyTexture);
  game->levelEnemyTexture = NULL;
  SDL_DestroyTexture(game->playerProjectileTexture);
  game->playerProjectileTexture = NULL;
  SDL_DestroyTexture(game->spriteSheetTexture);
  game->spriteSheetTexture = NULL;
  LevelFree(&game->level);
  SDL_DestroyRenderer(game->gameRenderer);
  game->gameRenderer = NULL;
  SDL_DestroyWindow(game->gameWindow);
  game->gameWindow = NULL;
  SDL_Quit();
}

void GameUpdate(Game *game, float deltaTime) {
  InputUpdate(&game->input);
  PlayerUpdate(&game->player, &game->input, &game->level, deltaTime);
  ProjectileHandlePlayerShooting(game->projectiles, &game->player,
                                 &game->input);
  ProjectileUpdateAll(game->projectiles, &game->level, deltaTime);
  LevelEnemyUpdateAll(game->level.levelEnemies,
                      LevelGetEnemyCount(&game->level), deltaTime);
  HandleProjectileEntityCollision(game->projectiles, game->level.levelEnemies,
                                  LevelGetEnemyCount(&game->level));
  float targetCameraX = game->player.entity.x - SCREEN_WIDTH / 2.0f;

  float targetCameraY = game->player.entity.y - SCREEN_HEIGHT / 2.0f;
  CameraUpdate(&game->camera, targetCameraX, targetCameraY, &game->level,
               deltaTime);
}
void GameRender(Game *game) {
  GraphicsClear(game->gameRenderer);
  GraphicsRenderLevel(&game->level, game->gameRenderer, &game->camera);
  GraphicsRenderPlayer(&game->player, game->gameRenderer, &game->camera,
                       game->spriteSheetTexture);
  ProjectileRenderAll(game->projectiles, game->gameRenderer, game->camera.x,
                      game->camera.y, game->playerProjectileTexture);
  LevelEnemyRenderAll(game->level.levelEnemies,
                      LevelGetEnemyCount(&game->level), game->gameRenderer,
                      &game->camera, game->levelEnemyTexture);
  GraphicsPresent(game->gameRenderer);
}
