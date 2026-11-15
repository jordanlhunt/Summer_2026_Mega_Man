#include "game.h"
#include "assetmanager.h"
#include "common.h"
#include "config.h"
#include "gameworld.h"
#include "graphics.h"
#include "level.h"
#include "levelEnemy.h"
#include "projectile.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

// Private Helpers

static bool GameLoadTexture(Game *game, const char *texturePath,
                            SDL_Texture **destination) {
  bool isDestinationFound = true;
  *destination = AssetManagerLoadTexture(&game->assetManager,
                                         game->gameRenderer, texturePath);
  if (*destination == NULL) {
    SDL_Log("Failed to load texture: %s", texturePath);
    isDestinationFound = false;
  }
  return isDestinationFound;
}

// End Private Helpers

bool GameInitialize(Game *game) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return false;
  }
  game->gameWindow = SDL_CreateWindow("SDL3 - Robot Hero Sara", SCREEN_WIDTH,
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
  // Load Assets
  AssetManagerInitialize(&game->assetManager);
  if (!GameLoadTexture(game, SARABOT_ASSET_PATH, &game->spriteSheetTexture) ||
      !GameLoadTexture(game, BIG_PROPELLER_BOT_ASSET_PATH,
                       &game->levelEnemyTexture) ||
      !GameLoadTexture(game, PLAYER_PROJECTILE_ASSET_PATH,
                       &game->playerProjectileTexture) ||
      !GameLoadTexture(game, HEALTH_PICKUP_ASSET_PATH,
                       &game->healthPickupTexture)) {
    goto textureLoadFailure;
  }

  game->gameWorld.player =
      (Player){.entity =
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
               .hitPoints = STARTING_HITPOINTS,
               .styleRow = STYLE_NES,
               .currentPlayerState = STATE_IDLE,
               .spriteSheetTexture = game->spriteSheetTexture};
  if (!GameWorldLoadLevel(&game->gameWorld, "assets/levels/testroom.txt")) {
    SDL_Log("Failed to load level file. %s", SDL_GetError());
    goto textureLoadFailure;
  }
  game->isRunning = true;
  game->previousTime = SDL_GetTicks();
  game->currentGameState = GAME_STATE_PLAYING;
  game->deathTimer = 0.0f;
  InputInitialize(&game->input);
  return true;

textureLoadFailure:
  GameShutdown(game);
  return false;
}
void GameShutdown(Game *game) {
  if (game == NULL) {
    return;
  }
  AssetManagerShutdown(&game->assetManager);
  game->levelEnemyTexture = NULL;
  game->playerProjectileTexture = NULL;
  game->spriteSheetTexture = NULL;
  game->healthPickupTexture = NULL;
  GameWorldShutdown(&game->gameWorld);
  if (game->gameRenderer != NULL) {
    SDL_DestroyRenderer(game->gameRenderer);
    game->gameRenderer = NULL;
  }
  if (game->gameWindow != NULL) {
    SDL_DestroyWindow(game->gameWindow);
    game->gameWindow = NULL;
  }
  SDL_Quit();
}
void GameUpdate(Game *game, float deltaTime) {
  switch (game->currentGameState) {
  case GAME_STATE_PLAYING: {
    InputUpdate(&game->input);
    GameWorldUpdate(&game->gameWorld, &game->input, deltaTime);
    if (game->gameWorld.player.hitPoints <= 0) {
      game->currentGameState = GAME_STATE_DYING;
      game->deathTimer = 1.0f;
    }
    break;
  }
  case GAME_STATE_DYING: {
    game->deathTimer -= deltaTime;
    if (game->deathTimer <= 0.0f) {
      game->currentGameState = GAME_STATE_DEAD;
    }
    break;
  }
  case GAME_STATE_DEAD: {
    char savedPath[MAX_LEVEL_PATH_LENGTH];
    strncpy(savedPath, game->gameWorld.currentLevelPath,
            MAX_LEVEL_PATH_LENGTH - 1);
    savedPath[MAX_LEVEL_PATH_LENGTH - 1] = '\0';
    GameWorldLoadLevel(&game->gameWorld, savedPath);
    game->gameWorld.player.hitPoints = STARTING_HITPOINTS;
    game->currentGameState = GAME_STATE_PLAYING;
    break;
  }
  default: {
    SDL_Log("Game Update Default Case Reached");
    break;
  }
  }
}
void GameRender(Game *game) {
  GraphicsClear(game->gameRenderer);
  GameWorldRender(&game->gameWorld, game->gameRenderer,
                  game->playerProjectileTexture, game->levelEnemyTexture,
                  game->healthPickupTexture);
  GraphicsPresent(game->gameRenderer);
}
