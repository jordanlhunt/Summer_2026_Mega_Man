#include "game.h"
bool GameInitialize(Game *game) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
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
  SDL_Surface *spriteSheetSurface = IMG_Load(SARABOT_ASSET_PATH);
  if (spriteSheetSurface == NULL) {
    SDL_Log("Unable to load sprite sheet: %s", SDL_GetError());
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    SDL_Quit();
    return false;
  } else {
    SDL_Log("Sprite sheet loaded successfully from: %s", SARABOT_ASSET_PATH);
  }
  game->spriteSheetTexture =
      SDL_CreateTextureFromSurface(game->gameRenderer, spriteSheetSurface);
  SDL_DestroySurface(spriteSheetSurface);
  if (game->spriteSheetTexture == NULL) {
    SDL_Log("Unable to create texture from surface: %s", SDL_GetError());
  }
  SDL_Surface *playerProjectileSurface = IMG_Load(PLAYER_PROJECTILE_ASSET_PATH);
  if (playerProjectileSurface == NULL) {
    SDL_Log("Unable to load player projectile sprite: %s", SDL_GetError());
    SDL_DestroyTexture(game->spriteSheetTexture);
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    SDL_Quit();
    return false;
  } else {
    SDL_Log("Player sprite loaded successfuly from: %s",
            PLAYER_PROJECTILE_ASSET_PATH);
  }
  game->playerProjectileTexture =
      SDL_CreateTextureFromSurface(game->gameRenderer, playerProjectileSurface);
  if (game->playerProjectileTexture == NULL) {
    SDL_Log("Unable to create texture from surface: %s", SDL_GetError());
  }

  game->player = (Player){.x = 0,
                          .y = 0,
                          .width = 32,
                          .height = 40,
                          .isFacingRight = true,
                          .hitPoints = 16,
                          .styleRow = STYLE_NES,
                          .currentPlayerState = STATE_IDLE};
  if (!LevelLoadFromFile(&game->level, "assets/levels/testroom.txt")) {
    SDL_Log("Failed to load level file.", SDL_GetError());
    // Clean up anything created so far:
    SDL_DestroyTexture(game->spriteSheetTexture);
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    SDL_Quit();
    return false;
  }
  game->camera.x = 0;
  game->camera.y = 0;
  game->player.x = game->level.playerSpawnX;
  game->player.y = game->level.playerSpawnY;
  game->isRunning = true;
  game->previousTime = SDL_GetTicks();

  memset(game->projectiles, 0, sizeof(game->projectiles));
  InputInitialize(&game->input);
  return true;
}
void GameShutdown(Game *game) {
  if (game->spriteSheetTexture) {
    SDL_DestroyTexture(game->spriteSheetTexture);
  }
  LevelFree(&game->level);
  SDL_DestroyRenderer(game->gameRenderer);
  SDL_DestroyWindow(game->gameWindow);
  SDL_Quit();
}
void GameUpdate(Game *game, float deltaTime) {
  InputUpdate(&game->input);
  PlayerUpdate(&game->player, &game->input, &game->level, deltaTime);
  ProjectileHandlePlayerShooting(game->projectiles, &game->player,
                                 &game->input);
  ProjectileUpdateAll(game->projectiles, &game->level, deltaTime);

  float targetCameraX = game->player.x - SCREEN_WIDTH / 2.0f;
  float targetCameraY = game->player.y - SCREEN_HEIGHT / 2.0f;
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
  GraphicsPresent(game->gameRenderer);
}
