#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <string.h>

#define SCREEN_WIDTH   1280
#define SCREEN_HEIGHT   720
#define TILE_SIZE        32
#define GRAVITY        2200.0f
#define MAX_FALL_SPEED  900.0f

/* Sprite sheet: 52x44 per frame, 8 cols x 6 rows */
#define SPR_W 52
#define SPR_H 44
#define SPR_COLS 8
#define SPR_ROWS 6

/* Style block starting rows */
#define STYLE_GBA 0
#define STYLE_BASE 2
#define STYLE_NES  4

/* Animation frame indices (column within a style block) */
enum {
    ANIM_IDLE_1 = 0,
    ANIM_TURN   = 1,
    ANIM_JUMP   = 2,
    ANIM_SHOOT  = 3,
    ANIM_SHOOT_FLASH = 4,
    ANIM_SHOOT_END   = 5,
    /* Row 1 */
    ANIM_RUN_1  = 0,
    ANIM_RUN_2  = 1,
    ANIM_RUN_3  = 2,
    ANIM_RUN_4  = 3,
    ANIM_RUN_5  = 4,
    ANIM_RUN_6  = 5,
    ANIM_SLIDE_1 = 6,
    ANIM_SLIDE_2 = 7,
};

typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_JUMPING,
    STATE_FALLING,
    STATE_WALL_SLIDING,
    STATE_DASHING,
    STATE_SLIDING,
} PlayerState;

typedef struct {
    float x, y;
    float vx, vy;
    float w, h;
    bool on_ground;
    bool facing_right;
    bool is_dashing;
    bool is_wall_sliding;
    bool can_wall_jump;
    int wall_direction;
    float dash_timer;
    float dash_cooldown;
    float wall_slide_timer;
    float anim_timer;
    int anim_frame;
    int health;
    PlayerState state;
    int style_row;
} Player;

typedef struct {
    int width, height;
    unsigned char *tiles;
} Level;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *sprite_sheet;
    Player player;
    Level level;
    float camera_x, camera_y;
    bool running;
    bool keys[SDL_SCANCODE_COUNT];
    Uint64 last_time;
} Game;

bool game_init(Game *g);
void game_shutdown(Game *g);
void game_update(Game *g, float dt);
void game_render(Game *g);

#endif
