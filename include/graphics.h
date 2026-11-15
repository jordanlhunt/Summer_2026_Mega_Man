#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "common.h"
#include "config.h"
#include "forwarddeclares.h"
#include <SDL3/SDL_render.h>

/* Rendering */
#define SPRITE_SCALE 2.0f
#define DEFAULT_ANIMATION_SPEED 8.0f
#define RUN_ANIMATION_SPEED 12.0f
#define RUN_ANIMATION_FRAMES 6

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
                          const Camera *camera);
void GraphicsPresent(SDL_Renderer *renderer);
#endif
