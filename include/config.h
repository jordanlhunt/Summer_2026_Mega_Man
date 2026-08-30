#ifndef CONFIG_H
#define CONFIG_H
/* -------------------------------------------------------------------- */
/* Window / world                                                       */
/* -------------------------------------------------------------------- */
#define SARABOT_ASSET_PATH "assets/sarabot-alpha.png"
#define PLAYER_PROJECTILE_ASSET_PATH "assets/shot.png"
#define BIG_PROPELLER_BOT_ASSET_PATH "assets/Big_Propeller_Bot.png"
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TILE_SIZE 32
/* -------------------------------------------------------------------- */
/* Physics                                                              */
/* -------------------------------------------------------------------- */
#define GRAVITY 2200.0f
#define MAX_FALL_SPEED 900.0f
/* -------------------------------------------------------------------- */
/* Sprite sheet layout: 52x44 per frame, 8 cols x 6 rows                */
/* -------------------------------------------------------------------- */
#define SPRITE_WIDTH 52
#define SPRITE_HEIGHT 44
#define SPRITE_COLUMNS 8
#define SPRITE_ROWS 6
/* Style block starting rows */
#define STYLE_GBA 0
#define STYLE_BASE 2
#define STYLE_NES 4
/* -------------------------------------------------------------------- */
/* Level Enemy sheet layout: 30x30 per frame, 6 cols x 1 rows                */
/* -------------------------------------------------------------------- */
#define LEVELENEMY_WIDTH 30
#define LEVELENEMY_HEIGHT 30
#define LEVELENEMY_COLUMNS 6
#define LEVELENEMY_ROWS 1
/* -------------------------------------------------------------------- */
/* Dashing                                                              */
/* -------------------------------------------------------------------- */
#define DASH_TRAIL_OFFSET 10.0f
/* -------------------------------------------------------------------- */
/* Camera                                                                */
/* -------------------------------------------------------------------- */
#define CAMERA_SMOOTHING 10.0f
#endif
