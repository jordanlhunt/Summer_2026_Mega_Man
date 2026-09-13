#ifndef GAME_H
#define GAME_H

#include "assetmanager.h"
#include "common.h"
#include "config.h"
#include "gameworld.h"
#include "graphics.h"
#include "input.h"
#include "level.h"
#include "player.h"
#include "projectile.h"
typedef struct Game {
  SDL_Window *gameWindow;
  SDL_Renderer *gameRenderer;
  AssetManager assetManager;
  SDL_Texture *spriteSheetTexture;
  SDL_Texture *playerProjectileTexture;
  SDL_Texture *levelEnemyTexture;
  GameWorld gameWorld;
  Input input;
  bool isRunning;
  Uint64 previousTime;
} Game;
bool GameInitialize(Game *game);
void GameShutdown(Game *game);
void GameUpdate(Game *game, float deltaTime);
void GameRender(Game *game);
bool GameChangeLevelAtDoor(Game *game, const char *levelPath,
                           int targetDoorTileX, int targetDoorTileY);
#endif
