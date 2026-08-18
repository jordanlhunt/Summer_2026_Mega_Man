#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <string.h>

#define SARABOT_ASSET_PATH "sarabot-alpha.png"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TILE_SIZE 32
#define GRAVITY 2200.0f
#define MAX_FALL_SPEED 900.0f

/* Sprite sheet: 52x44 per frame, 8 cols x 6 rows */
#define SPRITE_WIDTH 52
#define SPRITE_HEIGHT 44
#define SPRITE_COLUMNS 8
#define SPRITE_ROWS 6
/* Style block starting rows */
#define STYLE_GBA 0
#define STYLE_BASE 2
#define STYLE_NES 4
/* Animation frame indices (column within a style block) */
typedef enum ANIMATION_INDEX {
  ANIMATION_IDLE_1 = 0,
  ANIMATION_TURN = 1,
  ANIMATION_JUMP = 2,
  ANIMATION_SHOOT = 3,
  ANIMATION_SHOOT_FLASH = 4,
  ANIMATION_SHOOT_END = 5,
  /* Row 1 */
  ANIMATION_RUN_1 = 0,
  ANIMATION_RUN_2 = 1,
  ANIMATION_RUN_3 = 2,
  ANIMATION_RUN_4 = 3,
  ANIMATION_RUN_5 = 4,
  ANIMATION_RUN_6 = 5,
  ANIMATION_SLIDE_1 = 6,
  ANIMATION_SLIDE_2 = 7
} AnimationIndex;
typedef enum PLAYER_STATE {
  STATE_IDLE,
  STATE_RUNNING,
  STATE_JUMPING,
  STATE_FALLING,
  STATE_WALL_SLIDING,
  STATE_DASHING,
  STATE_SLIDING
} PlayerState;
typedef struct Player {
  float x;
  float y;
  float velocityX;
  float velocityY;
  float width;
  float height;
  bool isOnGround;
  bool isFacingRight;
  bool isDashing;
  bool canWallJump;
  int wallDirection;
  float dashTimer;
  float dashCooldown;
  float wallSlideTimer;
  float animationTimer;
  float animationFrameDuration;
  int hitPoints;
  PlayerState currentPlayerState;
  int styleRow;
} Player;
typedef struct Level {
  int width;
  int height;
  int *tiles;
} Level;
typedef struct Game {
  SDL_Window *gameWindow;
  SDL_Renderer *gameRenderer;
  SDL_Texture *spriteSheetTexture;
  Player player;
  Level level;
  float cameraX;
  float cameraY;
  bool isRunning;
  Uint64 previousTime;
  bool keys[SDL_SCANCODE_COUNT];
} Game;
bool GameInitialize(Game *game);
void GameShutdown(Game *game);
void GameUpdate(Game *game, float deltaTime);
void GameRender(Game *game);
#endif