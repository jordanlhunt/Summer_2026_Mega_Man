#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "config.h"
#include "level.h"
#include "player.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
typedef struct Camera {
  float x;
  float y;
} Camera;
/**
 * Smoothly moves the camera toward (targetX, targetY), clamped to the level
 * bounds.
 */
void CameraUpdate(Camera *camera, float targetX, float targetY,
                  const Level *level, float deltaTime);
/**
 *  Loads the sprite sheet from disk and uploads it as a texture.
 *  Returns NULL on failure (renderer is left untouched either way).
 */
SDL_Texture *GraphicsLoadSpriteSheet(SDL_Renderer *renderer, const char *path);
void GraphicsClear(SDL_Renderer *renderer);
void GraphicsRenderLevel(const Level *level, SDL_Renderer *renderer,
                         const Camera *camera);
void GraphicsRenderPlayer(const Player *player, SDL_Renderer *renderer,
                          const Camera *camera, SDL_Texture *sheet);
void GraphicsPresent(SDL_Renderer *renderer);
#endif
