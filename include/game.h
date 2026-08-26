#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "config.h"
#include "graphics.h"
#include "input.h"
#include "level.h"
#include "player.h"
#include "projectile.h"

typedef struct Game {
  SDL_Window *gameWindow;
  SDL_Renderer *gameRenderer;
  SDL_Texture *spriteSheetTexture;
  SDL_Texture *playerProjectileTexture;
  Player player;
  Level level;
  Camera camera;
  Input input;
  Projectile projectiles[MAX_PROJECTILES];
  bool isRunning;
  Uint64 previousTime;
} Game;
bool GameInitialize(Game *game);
void GameShutdown(Game *game);
void GameUpdate(Game *game, float deltaTime);
void GameRender(Game *game);

#endif
