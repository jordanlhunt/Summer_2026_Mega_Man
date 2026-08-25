#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "config.h"

/**
 * Input owns raw key state and turns into semnatic, per-frame booleans so
 * gameplay never touches SDL scancodes directly
 */
typedef struct Input {
  bool keys[SDL_SCANCODE_COUNT];
  // Edge-detection memory, previous frame's held state
  bool wasJumpKeyDown;
  bool wasDashKeyDown;
  bool isJumpJustPressed;
  bool isDashJustPressed;
  // Derived recomputed every call in InputUpate()
  bool moveLeft;
  bool moveRight;
  bool jumpDown;
  bool dashDown;
} Input;

void InputInitialize(Input *input);
/**
 * Feed raw SDL key up/down events that are polled from main()
 */
void InputHandleEvent(Input *input, const SDL_Event *event);

/**
 * Recompute the derived booleans above from current raw key state. Called once
 * per fixed logic update, right before PlayerUpdate()
 */
void InputUpdate(Input *input);
#endif
