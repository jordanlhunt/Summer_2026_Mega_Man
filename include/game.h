#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "config.h"
#include "graphics.h"
#include "input.h"
#include "level.h"
#include "player.h"

typedef struct Game {
  SDL_Window *gameWindow;
  SDL_Renderer *gameRenderer;
  SDL_Texture *spriteSheetTexture;
  Player player;
  Level level;
  Camera camera;
  Input input;
  bool isRunning;
  Uint64 previousTime;
} Game;
bool GameInitialize(Game *game);
void GameShutdown(Game *game);
void GameUpdate(Game *game, float deltaTime);
void GameRender(Game *game);
#endif
