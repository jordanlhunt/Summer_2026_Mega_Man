#include "game.h"
#include <SDL3/SDL_main.h>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  Game game = {0};
  if (GameInitialize(&game) == false) {
    return 1;
  }
  SDL_Event sdlEvent;
  const float FIXED_DELTATIME = 1.0f / 60.0f;
  float accumulator = 0.0f;
  while (game.isRunning == true) {
    Uint64 currentTime = SDL_GetTicks();
    float frameTime = (currentTime - game.previousTime) / 1000.0f;
    game.previousTime = currentTime;
    if (frameTime > 0.25f) {
      frameTime = 0.25f;
    }
    while (SDL_PollEvent(&sdlEvent)) {
      if (sdlEvent.type == SDL_EVENT_QUIT) {
        game.isRunning = false;
      }
      if (sdlEvent.type == SDL_EVENT_KEY_DOWN) {
        if (sdlEvent.key.scancode < SDL_SCANCODE_COUNT) {
          game.keys[sdlEvent.key.scancode] = true;
        }
        if (sdlEvent.key.scancode == SDL_SCANCODE_ESCAPE) {
          game.isRunning = false;
        }
      }
      if (sdlEvent.type == SDL_EVENT_KEY_UP) {
        if (sdlEvent.key.scancode < SDL_SCANCODE_COUNT) {
          game.keys[sdlEvent.key.scancode] = false;
        }
      }
    }
    accumulator += frameTime;
    while (accumulator >= FIXED_DELTATIME) {
      GameUpdate(&game, FIXED_DELTATIME);
      accumulator -= FIXED_DELTATIME;
    }
    GameRender(&game);
  }
  GameShutdown(&game);
  return 0;
}