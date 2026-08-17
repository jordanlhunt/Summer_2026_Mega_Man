#include "game.h"
#include "player.h"
#include "level.h"
#include <SDL3/SDL_image.h>

bool game_init(Game *g) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    if (!IMG_Init(IMG_INIT_PNG)) {
        SDL_Log("IMG_Init failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    g->window = SDL_CreateWindow("Sarabot Movement Test Room", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!g->window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    g->renderer = SDL_CreateRenderer(g->window, NULL);
    if (!g->renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        return false;
    }

    /* Load sprite sheet - try multiple paths for Linux Mint */
    const char *paths[] = {
        "sarabot-alpha.png",
        "./sarabot-alpha.png",
        "../sarabot-alpha.png",
        "assets/sarabot-alpha.png",
        "/usr/local/share/sarabot_test/sarabot-alpha.png",
        NULL
    };

    SDL_Surface *sheet_surface = NULL;
    for (int i = 0; paths[i] != NULL; i++) {
        sheet_surface = IMG_Load(paths[i]);
        if (sheet_surface) {
            SDL_Log("Loaded sprite sheet from: %s", paths[i]);
            break;
        }
    }

    if (!sheet_surface) {
        SDL_Log("Failed to load sprite sheet: %s", IMG_GetError());
        SDL_Log("Falling back to colored rectangles.");
        g->sprite_sheet = NULL;
    } else {
        g->sprite_sheet = SDL_CreateTextureFromSurface(g->renderer, sheet_surface);
        SDL_DestroySurface(sheet_surface);
        if (!g->sprite_sheet) {
            SDL_Log("Failed to create texture from surface: %s", SDL_GetError());
        }
    }

    /* Initialize player - spawn in the basic movement zone */
    g->player = (Player){
        .x = 160, .y = 500,
        .w = 32, .h = 40,
        .facing_right = true,
        .health = 16,
        .style_row = STYLE_GBA,
        .state = STATE_IDLE
    };

    level_generate_test_room(&g->level);

    g->camera_x = 0;
    g->camera_y = 0;
    g->running = true;
    g->last_time = SDL_GetTicks();

    memset(g->keys, 0, sizeof(g->keys));
    return true;
}

void game_shutdown(Game *g) {
    if (g->sprite_sheet) SDL_DestroyTexture(g->sprite_sheet);
    level_free(&g->level);
    SDL_DestroyRenderer(g->renderer);
    SDL_DestroyWindow(g->window);
    IMG_Quit();
    SDL_Quit();
}

void game_update(Game *g, float dt) {
    player_update(&g->player, g->keys, &g->level, dt);

    float target_cam_x = g->player.x - SCREEN_WIDTH / 2.0f;
    float target_cam_y = g->player.y - SCREEN_HEIGHT / 2.0f;

    float max_cam_x = g->level.width * TILE_SIZE - SCREEN_WIDTH;
    float max_cam_y = g->level.height * TILE_SIZE - SCREEN_HEIGHT;

    target_cam_x = SDL_clamp(target_cam_x, 0, max_cam_x > 0 ? max_cam_x : 0);
    target_cam_y = SDL_clamp(target_cam_y, 0, max_cam_y > 0 ? max_cam_y : 0);

    g->camera_x += (target_cam_x - g->camera_x) * 10.0f * dt;
    g->camera_y += (target_cam_y - g->camera_y) * 10.0f * dt;
}

void game_render(Game *g) {
    SDL_SetRenderDrawColor(g->renderer, 25, 25, 35, 255);
    SDL_RenderClear(g->renderer);

    level_render(&g->level, g->renderer, g->camera_x, g->camera_y);
    player_render(&g->player, g->renderer, g->camera_x, g->camera_y, g->sprite_sheet);

    SDL_RenderPresent(g->renderer);
}
