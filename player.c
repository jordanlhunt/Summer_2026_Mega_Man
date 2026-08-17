#include "player.h"
#include <math.h>

static void player_resolve_collision(Player *p, const Level *level, bool x_axis) {
    int start_tile_x = (int)(p->x / TILE_SIZE);
    int start_tile_y = (int)(p->y / TILE_SIZE);
    int end_tile_x = (int)((p->x + p->w) / TILE_SIZE);
    int end_tile_y = (int)((p->y + p->h) / TILE_SIZE);

    for (int ty = start_tile_y; ty <= end_tile_y; ty++) {
        for (int tx = start_tile_x; tx <= end_tile_x; tx++) {
            if (tx < 0 || tx >= level->width || ty < 0 || ty >= level->height)
                continue;

            if (level->tiles[ty * level->width + tx] == 0)
                continue;

            float tile_x = tx * TILE_SIZE;
            float tile_y = ty * TILE_SIZE;

            if (x_axis) {
                if (p->vx > 0) {
                    p->x = tile_x - p->w - 0.01f;
                } else if (p->vx < 0) {
                    p->x = tile_x + TILE_SIZE + 0.01f;
                }
                p->vx = 0;
            } else {
                if (p->vy > 0) {
                    p->y = tile_y - p->h - 0.01f;
                    p->on_ground = true;
                } else if (p->vy < 0) {
                    p->y = tile_y + TILE_SIZE + 0.01f;
                }
                p->vy = 0;
            }
            return;
        }
    }

    if (!x_axis && p->vy > 0) {
        p->on_ground = false;
    }
}

void player_update(Player *p, const bool *keys, const Level *level, float dt) {
    bool want_dash = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_Z];
    bool want_jump = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_X];
    bool move_left = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A];
    bool move_right = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];

    static float jump_buffer = 0;
    static float coyote_timer = 0;
    static bool jump_held = false;
    static bool was_on_ground = false;

    if (want_jump && !jump_held) {
        jump_buffer = JUMP_BUFFER_TIME;
    }
    jump_held = want_jump;
    if (jump_buffer > 0) jump_buffer -= dt;

    if (p->on_ground) {
        coyote_timer = COYOTE_TIME;
        was_on_ground = true;
    } else if (was_on_ground) {
        coyote_timer -= dt;
        if (coyote_timer <= 0) was_on_ground = false;
    }

    /* Dash handling */
    if (want_dash && p->dash_cooldown <= 0 && !p->is_dashing && p->on_ground) {
        p->is_dashing = true;
        p->dash_timer = DASH_DURATION;
        p->dash_cooldown = DASH_COOLDOWN;
        p->vx = p->facing_right ? DASH_SPEED : -DASH_SPEED;
        p->vy = 0;
        p->state = STATE_DASHING;
    }

    if (p->is_dashing) {
        p->dash_timer -= dt;
        if (p->dash_timer <= 0) {
            p->is_dashing = false;
            p->vx *= 0.3f;
        }
        p->vy += GRAVITY * 0.2f * dt;
        p->anim_timer += dt;

        p->x += p->vx * dt;
        player_resolve_collision(p, level, true);
        p->y += p->vy * dt;
        player_resolve_collision(p, level, false);
        return;
    } else {
        p->dash_cooldown -= dt;
    }

    /* Normal movement */
    float target_vx = 0;
    if (move_left) {
        target_vx = -PLAYER_SPEED;
        if (!p->is_wall_sliding) p->facing_right = false;
    }
    if (move_right) {
        target_vx = PLAYER_SPEED;
        if (!p->is_wall_sliding) p->facing_right = true;
    }

    float accel = p->on_ground ? 25.0f : 12.0f;
    p->vx += (target_vx - p->vx) * accel * dt;

    /* Wall sliding detection */
    p->is_wall_sliding = false;
    p->can_wall_jump = false;
    p->wall_direction = 0;

    if (!p->on_ground && p->vy > 0) {
        int tile_x_left = (int)(p->x / TILE_SIZE);
        int tile_x_right = (int)((p->x + p->w) / TILE_SIZE);
        int tile_y_top = (int)(p->y / TILE_SIZE);
        int tile_y_bot = (int)((p->y + p->h) / TILE_SIZE);

        bool wall_on_left = false, wall_on_right = false;
        for (int ty = tile_y_top; ty <= tile_y_bot && ty < level->height; ty++) {
            if (ty < 0) continue;
            if (tile_x_left >= 0 && tile_x_left < level->width &&
                level->tiles[ty * level->width + tile_x_left] != 0) {
                wall_on_left = true;
            }
            if (tile_x_right >= 0 && tile_x_right < level->width &&
                level->tiles[ty * level->width + tile_x_right] != 0) {
                wall_on_right = true;
            }
        }

        if (move_left && wall_on_left) {
            p->is_wall_sliding = true;
            p->wall_direction = -1;
            p->facing_right = true;
        } else if (move_right && wall_on_right) {
            p->is_wall_sliding = true;
            p->wall_direction = 1;
            p->facing_right = false;
        }

        if (p->is_wall_sliding) {
            p->vy = fminf(p->vy, WALL_SLIDE_SPEED);
            p->wall_slide_timer += dt;
            p->can_wall_jump = true;
        } else {
            p->wall_slide_timer = 0;
        }
    }

    /* Jumping */
    if (jump_buffer > 0 && (p->on_ground || coyote_timer > 0)) {
        p->vy = -JUMP_FORCE;
        p->on_ground = false;
        coyote_timer = 0;
        jump_buffer = 0;
        p->state = STATE_JUMPING;
    }

    /* Wall jump */
    if (jump_buffer > 0 && p->can_wall_jump) {
        p->vy = -WALL_JUMP_FORCE_Y;
        p->vx = p->wall_direction * WALL_JUMP_FORCE_X;
        p->is_wall_sliding = false;
        p->can_wall_jump = false;
        jump_buffer = 0;
        p->state = STATE_JUMPING;
        p->facing_right = (p->wall_direction > 0);
    }

    /* Variable jump height */
    if (!want_jump && p->vy < -180.0f && !p->is_wall_sliding) {
        p->vy *= 0.88f;
    }

    /* Gravity */
    if (!p->is_wall_sliding) {
        p->vy += GRAVITY * dt;
    } else {
        p->vy += GRAVITY * 0.3f * dt;
    }
    if (p->vy > MAX_FALL_SPEED) p->vy = MAX_FALL_SPEED;

    /* Move and resolve collisions */
    p->x += p->vx * dt;
    player_resolve_collision(p, level, true);

    p->y += p->vy * dt;
    player_resolve_collision(p, level, false);

    /* Update state for animation */
    if (p->is_dashing) {
        p->state = STATE_DASHING;
    } else if (p->is_wall_sliding) {
        p->state = STATE_WALL_SLIDING;
    } else if (!p->on_ground) {
        p->state = (p->vy < 0) ? STATE_JUMPING : STATE_FALLING;
    } else if (fabsf(p->vx) > 10.0f) {
        p->state = STATE_RUNNING;
    } else {
        p->state = STATE_IDLE;
    }

    p->anim_timer += dt;
}

void player_render(const Player *p, SDL_Renderer *renderer, float cam_x, float cam_y, SDL_Texture *sheet) {
    float draw_x = p->x - cam_x - (SPR_W - p->w) / 2.0f;
    float draw_y = p->y - cam_y - (SPR_H - p->h);

    SDL_FRect dst = { draw_x, draw_y, SPR_W * 2.0f, SPR_H * 2.0f };

    if (sheet) {
        int frame_col = 0;
        int frame_row = p->style_row;
        float anim_speed = 8.0f;

        switch (p->state) {
            case STATE_IDLE:
                frame_col = ANIM_IDLE_1;
                break;
            case STATE_RUNNING:
                anim_speed = 12.0f;
                frame_col = ANIM_RUN_1 + ((int)(p->anim_timer * anim_speed) % 6);
                frame_row = p->style_row + 1;
                break;
            case STATE_JUMPING:
                frame_col = ANIM_JUMP;
                break;
            case STATE_FALLING:
                frame_col = ANIM_JUMP;
                break;
            case STATE_WALL_SLIDING:
                frame_col = ANIM_TURN;
                break;
            case STATE_DASHING:
                frame_col = ANIM_SLIDE_1;
                frame_row = p->style_row + 1;
                break;
            case STATE_SLIDING:
                frame_col = ANIM_SLIDE_1;
                frame_row = p->style_row + 1;
                break;
        }

        SDL_FRect src = {
            frame_col * SPR_W,
            frame_row * SPR_H,
            SPR_W,
            SPR_H
        };

        SDL_FlipMode flip = p->facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

        /* Dash trail effect */
        if (p->is_dashing) {
            SDL_SetTextureAlphaMod(sheet, 80);
            SDL_FRect trail_dst = dst;
            trail_dst.x -= p->facing_right ? 30 : -30;
            SDL_RenderTextureRotated(renderer, sheet, &src, &trail_dst, 0, NULL, flip);
            SDL_SetTextureAlphaMod(sheet, 255);
        }

        SDL_RenderTextureRotated(renderer, sheet, &src, &dst, 0, NULL, flip);
    } else {
        SDL_FRect rect = { p->x - cam_x, p->y - cam_y, p->w, p->h };
        switch (p->state) {
            case STATE_DASHING: SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255); break;
            case STATE_WALL_SLIDING: SDL_SetRenderDrawColor(renderer, 255, 200, 50, 255); break;
            case STATE_JUMPING: SDL_SetRenderDrawColor(renderer, 50, 150, 255, 255); break;
            default: SDL_SetRenderDrawColor(renderer, 50, 200, 100, 255); break;
        }
        SDL_RenderFillRect(renderer, &rect);
    }
}
