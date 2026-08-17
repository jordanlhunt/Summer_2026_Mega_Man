#ifndef LEVEL_H
#define LEVEL_H

#include "game.h"

void level_generate_test_room(Level *l);
void level_free(Level *l);
void level_render(const Level *l, SDL_Renderer *renderer, float cam_x,
                  float cam_y);

#endif
