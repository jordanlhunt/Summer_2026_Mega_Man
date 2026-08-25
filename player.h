#ifndef PLAYER_H
#define PLAYER_H
#include "collision.h"
#include "config.h"
#include "input.h"
#include "level.h"
#include <math.h>
#include <stdbool.h>
/* Tuning constants */
#define PLAYER_SPEED 220.0f
#define JUMP_FORCE 520.0f
#define WALL_JUMP_FORCE_X 350.0f
#define WALL_JUMP_FORCE_Y 480.0f
#define DASH_SPEED 550.0f
#define DASH_DURATION 0.18f
#define DASH_COOLDOWN 0.25f
#define WALL_SLIDE_SPEED 120.0f
#define COYOTE_TIME 0.08f
#define JUMP_BUFFER_TIME 0.1f
#define GROUND_ACCELERATION 25.0f
#define AIR_ACCELERATION 12.0f
#define VARIABLE_JUMP_THRESHOLD -180.0f
#define VARIABLE_JUMP_MULTIPLIER 0.88f
#define WALL_SLIDE_GRAVITY_SCALE 0.3f
#define DASH_GRAVITY_SCALE 0.2f
/* Animation frame indices (column within a style block) */
typedef enum ANIMATION_INDEX {
  ANIMATION_IDLE_1,
  ANIMATION_TURN,
  ANIMATION_JUMP,
  ANIMATION_SHOOT,
  ANIMATION_SHOOT_FLASH,
  ANIMATION_SHOOT_END, /* Row 1 */
  ANIMATION_RUN_1,
  ANIMATION_RUN_2,
  ANIMATION_RUN_3,
  ANIMATION_RUN_4,
  ANIMATION_RUN_5,
  ANIMATION_RUN_6,
  ANIMATION_SLIDE_1,
  ANIMATION_SLIDE_2
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
  bool isWallSliding;
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
  float jumpBufferTimer;
  float coyoteTimer;
} Player;
void PlayerUpdate(Player *player, const Input *Input, const Level *level,
                  float deltaTime);
#endif
