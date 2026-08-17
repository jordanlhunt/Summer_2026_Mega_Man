#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

#define PLAYER_SPEED       220.0f
#define JUMP_FORCE         520.0f
#define WALL_JUMP_FORCE_X  350.0f
#define WALL_JUMP_FORCE_Y  480.0f
#define DASH_SPEED         550.0f
#define DASH_DURATION      0.18f
#define DASH_COOLDOWN      0.25f
#define WALL_SLIDE_SPEED   120.0f
#define COYOTE_TIME        0.08f
#define JUMP_BUFFER_TIME   0.1f

void player_update(Player *p, const bool *keys, const Level *level, float dt);
void player_render(const Player *p, SDL_Renderer *renderer, float cam_x, float cam_y, SDL_Texture *sheet);

#endif
