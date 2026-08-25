#include "input.h"

void InputInitialize(Input *input) { memset(input, 0, sizeof(*input)); }
void InputHandleEvent(Input *input, const SDL_Event *event) {
  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.scancode < SDL_SCANCODE_COUNT) {
      input->keys[event->key.scancode] = true;
    }
  } else if (event->type == SDL_EVENT_KEY_UP) {
    if (event->key.scancode < SDL_SCANCODE_COUNT) {
      input->keys[event->key.scancode] = false;
    }
  }
}
void InputUpdate(Input *input) {
  input->moveLeft =
      input->keys[SDL_SCANCODE_LEFT] || input->keys[SDL_SCANCODE_A];
  input->moveRight =
      input->keys[SDL_SCANCODE_RIGHT] || input->keys[SDL_SCANCODE_D];
  input->jumpDown =
      input->keys[SDL_SCANCODE_SPACE] || input->keys[SDL_SCANCODE_J];
  input->dashDown =
      input->keys[SDL_SCANCODE_LSHIFT] || input->keys[SDL_SCANCODE_Z];
  input->shootDown = input->keys[SDL_SCANCODE_K] || input->keys[SDL_SCANCODE_X];
  input->isJumpJustPressed = input->jumpDown && !input->wasJumpKeyDown;
  input->isDashJustPressed = input->dashDown && !input->wasDashKeyDown;
  input->isShootJustPressed = input->shootDown && !input->wasShootKeyDown;
  input->wasJumpKeyDown = input->jumpDown;
  input->wasDashKeyDown = input->dashDown;
  input->wasShootKeyDown = input->shootDown;
}
