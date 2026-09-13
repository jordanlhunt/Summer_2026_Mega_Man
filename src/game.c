#include "game.h"
#include "assetmanager.h"
#include "common.h"
#include "config.h"
#include "gameworld.h"
#include "graphics.h"
#include "level.h"
#include "levelEnemy.h"
#include "projectile.h"
#include <SDL3/SDL_stdinc.h>
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
  // Load Assets
  AssetManagerInitialize(&game->assetManager);
  game->spriteSheetTexture = AssetManagerLoadTexture(
      &game->assetManager, game->gameRenderer, SARABOT_ASSET_PATH);
  if (!game->spriteSheetTexture) {
    SDL_Log("Failed to load spritesheet texture");
    AssetManagerShutdown(&game->assetManager);
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    return false;
  }
  game->levelEnemyTexture = AssetManagerLoadTexture(
      &game->assetManager, game->gameRenderer, BIG_PROPELLER_BOT_ASSET_PATH);
  if (!game->levelEnemyTexture) {
    SDL_Log("Failed to load spritesheet texture");
    AssetManagerShutdown(&game->assetManager);
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    return false;
  }
  game->playerProjectileTexture = AssetManagerLoadTexture(
      &game->assetManager, game->gameRenderer, PLAYER_PROJECTILE_ASSET_PATH);
  if (!game->playerProjectileTexture) {
    SDL_Log("Failed to load player projectile texture");
    AssetManagerShutdown(&game->assetManager);
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    return false;
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
               .hitPoints = 16,
               .styleRow = STYLE_NES,
               .currentPlayerState = STATE_IDLE,
               .spriteSheetTexture = game->spriteSheetTexture};
  if (!GameWorldLoadLevel(&game->gameWorld, "assets/levels/testroom.txt")) {
    SDL_Log("Failed to load level file. %s", SDL_GetError());
    AssetManagerShutdown(&game->assetManager);
    SDL_DestroyRenderer(game->gameRenderer);
    SDL_DestroyWindow(game->gameWindow);
    SDL_Quit();
    return false;
  }
  game->isRunning = true;
  game->previousTime = SDL_GetTicks();
  InputInitialize(&game->input);
  return true;
}
void GameShutdown(Game *game) {
  if (game == NULL) {
    return;
  }
  AssetManagerShutdown(&game->assetManager);
  game->levelEnemyTexture = NULL;
  game->playerProjectileTexture = NULL;
  game->spriteSheetTexture = NULL;
  GameWorldShutdown(&game->gameWorld);
  SDL_DestroyRenderer(game->gameRenderer);
  game->gameRenderer = NULL;
  SDL_DestroyWindow(game->gameWindow);
  game->gameWindow = NULL;
  SDL_Quit();
}
void GameUpdate(Game *game, float deltaTime) {
  InputUpdate(&game->input);
  GameWorldUpdate(&game->gameWorld, &game->input, deltaTime);
}
void GameRender(Game *game) {
  GraphicsClear(game->gameRenderer);
  GameWorldRender(&game->gameWorld, game->gameRenderer,
                  game->playerProjectileTexture, game->levelEnemyTexture);
  GraphicsPresent(game->gameRenderer);
}
bool GameChangeLevelAtDoor(Game *game, const char *levelPath,
                           int targetDoorTileX, int targetDoorTileY) {
  Level newLevelToLoad = {0};
  GameWorld *gameWorld = &game->gameWorld;
  if (!LevelLoadFromFile(&newLevelToLoad, levelPath)) {
    SDL_Log("Failed to load level '%s'", levelPath);
    return false;
  }

  const DoorTransition *targetDoor =
      LevelFindDoorAtTile(&newLevelToLoad, targetDoorTileX, targetDoorTileY);
  if (targetDoor == NULL) {
    SDL_Log("Level '%s' has no door at (%d, %d)", levelPath, targetDoorTileX,
            targetDoorTileY);
    LevelFree(&newLevelToLoad);
    return false;
  }
  LevelFree(&gameWorld->level);
  gameWorld->level = newLevelToLoad;
  int spawnTileX = targetDoor->tileX + targetDoor->spawnOffsetTileX;
  int spawnTileY = targetDoor->tileY + targetDoor->spawnOffsetTileY;
  gameWorld->player.entity.x =
      spawnTileX * TILE_SIZE +
      (TILE_SIZE - gameWorld->player.entity.width) * .5f;
  gameWorld->player.entity.y =
      spawnTileY * TILE_SIZE + TILE_SIZE - gameWorld->player.entity.height;
  gameWorld->player.entity.velocityX = 0.0f;
  gameWorld->player.entity.velocityY = 0.0f;
  gameWorld->player.isDashing = false;
  gameWorld->player.isWallSliding = false;
  gameWorld->player.entity.isOnGround = false;
  gameWorld->player.dashTimer = 0.0f;
  gameWorld->player.dashCooldown = 0.0f;
  gameWorld->player.wallSlideTimer = 0.0f;
  gameWorld->player.coyoteTimer = 0.0f;
  gameWorld->player.jumpBufferTimer = 0.0f;
  gameWorld->player.shootAnimationTimer = 0.0f;
  gameWorld->player.animationTimer = 0.0f;
  gameWorld->player.currentPlayerState = STATE_IDLE;
  memset(gameWorld->projectiles, 0, sizeof(gameWorld->projectiles));
  float maxX = LevelGetWidthPixels(&gameWorld->level) - SCREEN_WIDTH;
  float maxY = LevelGetHeightPixels(&gameWorld->level) - SCREEN_HEIGHT;
  if (maxX < 0.0f) {
    maxX = 0.0f;
  }
  if (maxY < 0.0f) {
    maxY = 0.0f;
  }
  gameWorld->camera.x =
      SDL_clamp(gameWorld->player.entity.x - SCREEN_WIDTH / 2.0f, 0.0f, maxX);
  gameWorld->camera.y =
      SDL_clamp(gameWorld->player.entity.y - SCREEN_HEIGHT / 2.0f, 0.0f, maxY);
  return true;
}