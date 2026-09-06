#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "common.h"
#include "config.h"

#include "forwarddeclares.h"

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
