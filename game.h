#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "graphics.h"
#include "input.h"
#include "level.h"
#include "player.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct Game {
  SDL_Window *gameWindow;
  SDL_Renderer *gameRenderer;
  SDL_Texture *spriteSheetTexture;
  Player player;
  Level level;
  float cameraX;
  float cameraY;
  bool isRunning;
  Uint64 previousTime;
  bool keys[SDL_SCANCODE_COUNT];
} Game;
bool GameInitialize(Game *game);
void GameShutdown(Game *game);
void GameUpdate(Game *game, float deltaTime);
void GameRender(Game *game);
#endif
