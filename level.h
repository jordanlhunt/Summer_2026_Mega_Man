#ifndef LEVEL_H
#define LEVEL_H
#include "game.h"
#include <stdlib.h>
#include <string.h>
void LevelGenerateTestRoom(Level *level);
void LevelFree(Level *level);
void LevelRender(const Level *level, SDL_Renderer *renderer, float cameraX,
                 float cameraY);
#endif