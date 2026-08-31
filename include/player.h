#ifndef PLAYER_H
#define PLAYER_H

#include "collision.h"
#include "common.h"
#include "config.h"
#include "entity.h"

typedef struct Input Input;
typedef struct Level Level;

/* Tuning constants */
#define AIR_ACCELERATION 12.0f
#define COYOTE_TIME 0.08f
#define DASH_COOLDOWN 0.25f
#define DASH_DURATION 0.18f
#define DASH_GRAVITY_SCALE 0.2f
#define DASH_SPEED 550.0f
#define GROUND_ACCELERATION 25.0f
#define JUMP_BUFFER_TIME 0.1f
#define JUMP_FORCE 520.0f
#define PLAYER_SPEED 220.0f
#define SHOOT_ANIMATION_DURATION 0.18f
#define SHOOT_FLASH_DURATION 0.05f
#define SHOOT_START_DURATION 0.10f
#define VARIABLE_JUMP_MULTIPLIER 0.88f
#define VARIABLE_JUMP_THRESHOLD -180.0f
#define WALL_JUMP_FORCE_X 350.0f
#define WALL_JUMP_FORCE_Y 480.0f
#define WALL_SLIDE_GRAVITY_SCALE 0.3f
#define WALL_SLIDE_SPEED 120.0f
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
  bool canWallJump;
  bool isDashing;
  bool isFacingRight;
  bool isOnGround;
  bool isWallSliding;
  float animationTimer;
  float shootAnimationTimer;
  float coyoteTimer;
  float dashCooldown;
  float dashTimer;
  Entity entity;
  float jumpBufferTimer;
  float shootCooldown;
  float wallSlideTimer;

  int hitPoints;
  int styleRow;
  int wallDirection;
  PlayerState currentPlayerState;
} Player;
void PlayerUpdate(Player *player, const Input *input, const Level *level,
                  float deltaTime);
#endif
