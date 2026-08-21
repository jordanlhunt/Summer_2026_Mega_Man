#ifndef LEVEL_H
#define LEVEL_H
#include "game.h"
#include <SDL3/SDL_rect.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void LevelFree(Level *level);
void LevelRender(const Level *level, SDL_Renderer *renderer, float cameraX,
                 float cameraY);
bool LevelLoadFromFile(Level *level, const char *filepath);
#endif
