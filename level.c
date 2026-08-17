#include "level.h"
#include <stdlib.h>
#include <string.h>

void level_generate_test_room(Level *l) {
    l->width = 40;
    l->height = 24;
    l->tiles = calloc(l->width * l->height, sizeof(unsigned char));

    memset(l->tiles, 0, l->width * l->height);

    /* Floor across the entire bottom */
    for (int x = 0; x < l->width; x++) {
        l->tiles[(l->height - 2) * l->width + x] = 1;
        l->tiles[(l->height - 1) * l->width + x] = 1;
    }

    /* === ZONE 1: SPAWN / BASIC MOVEMENT (left side) === */
    /* A few small platforms to practice basic jumps */
    for (int x = 3; x < 7; x++) {
        l->tiles[(l->height - 5) * l->width + x] = 1;
    }
    for (int x = 9; x < 13; x++) {
        l->tiles[(l->height - 7) * l->width + x] = 1;
    }

    /* === ZONE 2: WALL SLIDE / WALL JUMP (center) === */
    /* Left wall of the shaft */
    for (int y = 6; y < 16; y++) {
        l->tiles[y * l->width + 18] = 1;
    }
    /* Right wall of the shaft */
    for (int y = 6; y < 16; y++) {
        l->tiles[y * l->width + 24] = 1;
    }
    /* Mid-shaft rest platform */
    for (int x = 20; x <= 22; x++) {
        l->tiles[11 * l->width + x] = 1;
    }
    /* Top exit platform */
    for (int x = 18; x <= 24; x++) {
        l->tiles[5 * l->width + x] = 1;
    }

    /* === ZONE 3: ENEMY SPAWN ARENA (right side) === */
    /* Raised platform arena for enemy testing */
    for (int x = 28; x < 38; x++) {
        l->tiles[(l->height - 4) * l->width + x] = 1;
    }
    /* A small upper platform in the arena */
    for (int x = 30; x < 36; x++) {
        l->tiles[(l->height - 8) * l->width + x] = 1;
    }

    /* Side walls */
    for (int y = 0; y < l->height; y++) {
        l->tiles[y * l->width] = 1;
        l->tiles[y * l->width + l->width - 1] = 1;
    }
}

void level_free(Level *l) {
    free(l->tiles);
    l->tiles = NULL;
}

void level_render(const Level *l, SDL_Renderer *renderer, float cam_x, float cam_y) {
    int start_x = (int)(cam_x / TILE_SIZE) - 1;
    int start_y = (int)(cam_y / TILE_SIZE) - 1;
    int end_x = start_x + (SCREEN_WIDTH / TILE_SIZE) + 3;
    int end_y = start_y + (SCREEN_HEIGHT / TILE_SIZE) + 3;

    for (int y = start_y; y < end_y && y < l->height; y++) {
        for (int x = start_x; x < end_x && x < l->width; x++) {
            if (x < 0 || y < 0) continue;

            if (l->tiles[y * l->width + x] == 0) continue;

            SDL_FRect tile = {
                x * TILE_SIZE - cam_x,
                y * TILE_SIZE - cam_y,
                TILE_SIZE, TILE_SIZE
            };

            bool is_floor = (y >= l->height - 3);
            bool is_wall = (x == 0 || x == l->width - 1);

            if (y + 1 < l->height && l->tiles[(y + 1) * l->width + x] == 0 && !is_floor) {
                SDL_SetRenderDrawColor(renderer, 80, 70, 60, 255);
            } else if (is_wall) {
                SDL_SetRenderDrawColor(renderer, 60, 55, 50, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 45, 40, 35, 255);
            }

            SDL_RenderFillRect(renderer, &tile);

            SDL_SetRenderDrawColor(renderer, 100, 90, 80, 255);
            SDL_RenderRect(renderer, &tile);
        }
    }
}
