#include "game.h"
#include "level.h"
#include "player.h"
#include <SDL3_image/SDL_image.h>
bool GameInitialize(Game *game) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return false;
  }
  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
    SDL_Log("Unable to initialize SDL_image: %s", IMG_GetError());
    SDL_Quit();
    return false;
  }
  game->gameWindow = SDL_CreateWindow("SDL3 - Robot Hero Moby", SCREEN_WIDTH,
                                      SCREEN_HEIGHT, 0);
  if (game->gameWindow == NULL) {
    SDL_Log("Unable to create window: %s", SDL_GetError());
    IMG_Quit();
    SDL_Quit();
    return false;
  }
  game->gameRenderer = SDL_CreateRenderer(game->gameWindow, NULL);
  if (!SDL_SetRenderVSync(game->gameRenderer, 1)) {
    SDL_Log("Warning: VSync not enabled: %s", SDL_GetError());
  }
  if (game->gameRenderer == NULL) {
    SDL_Log("Unable to create renderer: %s", SDL_GetError());
    SDL_DestroyWindow(game->gameWindow);
    IMG_Quit();
    SDL_Quit();
    return false;
  }
  SDL_Surface *spriteSheetSurface = IMG_Load(SARABOT_ASSET_PATH);
  if (spriteSheetSurface == NULL) {
    SDL_Log("Unable to load sprite sheet: %s", IMG_GetError());
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    IMG_Quit();
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
  game->player = (Player){.x = 160,
                          .y = 500,
                          .width = 32,
                          .height = 40,
                          .isFacingRight = true,
                          .hitPoints = 16,
                          .styleRow = STYLE_GBA,
                          .currentPlayerState = STATE_IDLE};
  LevelGenerateTestRoom(&game->level);
  game->cameraX = 0;
  game->cameraY = 0;
  game->isRunning = true;
  game->previousTime = SDL_GetTicks();
  memset(game->keys, 0, sizeof(game->keys));
  return true;
}
void GameShutdown(Game *game) {
  if (game->spriteSheetTexture) {
    SDL_DestroyTexture(game->spriteSheetTexture);
  }
  LevelFree(&game->level);
  SDL_DestroyRenderer(game->gameRenderer);
  SDL_DestroyWindow(game->gameWindow);
  IMG_Quit();
  SDL_Quit();
}
void GameUpdate(Game *game, float deltaTime) {
  PlayerUpdate(&game->player, game->keys, &game->level, deltaTime);
  float targetCameraX = game->player.x - SCREEN_WIDTH / 2.0f;
  float targetCameraY = game->player.y - SCREEN_HEIGHT / 2.0f;
  float maxCameraX = game->level.width * TILE_SIZE - SCREEN_WIDTH;
  float maxCameraY = game->level.height * TILE_SIZE - SCREEN_HEIGHT;
  float clampMaxX = maxCameraX;
  if (clampMaxX < 0) {
    clampMaxX = 0;
  }
  float clampMaxY = maxCameraY;
  if (clampMaxY < 0) {
    clampMaxY = 0;
  }
  targetCameraX = SDL_clamp(targetCameraX, 0, clampMaxX);
  targetCameraY = SDL_clamp(targetCameraY, 0, clampMaxY);
  game->cameraX += (targetCameraX - game->cameraX) * 10.0f * (deltaTime);
  game->cameraY += (targetCameraY - game->cameraY) * 10.0f * (deltaTime);
}
void GameRender(Game *game) {
  SDL_SetRenderDrawColor(game->gameRenderer, 147, 204, 234, 255);
  SDL_RenderClear(game->gameRenderer);
  LevelRender(&game->level, game->gameRenderer, game->cameraX, game->cameraY);
  PlayerRender(&game->player, game->gameRenderer, game->cameraX, game->cameraY,
               game->spriteSheetTexture);
  SDL_RenderPresent(game->gameRenderer);
}