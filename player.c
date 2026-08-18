#include "player.h"
static void PlayerResolveCollision(Player *player, const Level *level, bool xAxis)
{
    int startTileX = (int)(player->x / TILE_SIZE);
    int startTileY = (int)(player->y / TILE_SIZE);
    int endTileX = (int)((player->x + player->width) / TILE_SIZE);
    int endTileY = (int)((player->y + player->height) / TILE_SIZE);
    for (int tileY = startTileY; tileY <= endTileY; tileY++)
    {
        for (int tileX = startTileX; tileX <= endTileX; tileX++)
        {
            if (tileX < 0 || tileX >= level->width || tileY < 0 || tileY >= level->height)
            {
                continue;
            }
            if (level->tiles[tileY * level->width + tileX] == 0)
            {
                continue;
            }
            float tile_x = tileX * TILE_SIZE;
            float tile_y = tileY * TILE_SIZE;
            if (xAxis)
            {
                if (player->velocityX > 0)
                {
                    player->x = tile_x - player->width - 0.01f;
                }
                else if (player->velocityX < 0)
                {
                    player->x = tile_x + TILE_SIZE + 0.01f;
                }
                player->velocityX = 0;
            }
            else
            {
                if (player->velocityY > 0)
                {
                    player->y = tile_y - player->height - 0.01f;
                    player->isOnGround = true;
                }
                else if (player->velocityY < 0)
                {
                    player->y = tile_y + TILE_SIZE + 0.01f;
                }
                player->velocityY = 0;
            }
            return;
        }
    }
    if (!xAxis && player->velocityY > 0)
    {
        player->isOnGround = false;
    }
}
void PlayerUpdate(Player *player, const bool *keys, const Level *level, float deltaTime)
{
    bool dashKeyDown = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_Z];
    bool jumpKeyDown = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_J];
    bool moveLeftKeyDown = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A];
    bool moveRightKeyDown = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];
    static bool dashKeyWasDown = false;
    static bool jumpKeyWasDown = false;

    bool dashKeyJustPressed = dashKeyDown && !dashKeyWasDown;
    bool jumpKeyJustPressed = jumpKeyDown && !jumpKeyWasDown;

    dashKeyWasDown = dashKeyDown;
    jumpKeyWasDown = jumpKeyDown;

    static float jumpBuffer = 0.0f;
    static float coyoteTimer = 0.0f;

    if (jumpKeyJustPressed)
    {
        jumpBuffer = JUMP_BUFFER_TIME;
    }

    if (jumpBuffer > 0.0f)
    {
        jumpBuffer -= deltaTime;

        if (jumpBuffer < 0.0f)
        {
            jumpBuffer = 0.0f;
        }
    }

    if (player->isOnGround)
    {
        coyoteTimer = COYOTE_TIME;
    }
    else if (coyoteTimer > 0.0f)
    {
        coyoteTimer -= deltaTime;

        if (coyoteTimer < 0.0f)
        {
            coyoteTimer = 0.0f;
        }
    }
    /* Dash handling   */
    if (dashKeyJustPressed && player->dashCooldown <= 0.0f && !player->isDashing && player->isOnGround)
    {
        player->isDashing = true;
        player->dashTimer = DASH_DURATION;
        player->dashCooldown = DASH_COOLDOWN;

        if (player->isFacingRight)
        {
            player->velocityX = DASH_SPEED;
        }
        else
        {
            player->velocityX = -DASH_SPEED;
        }

        player->velocityY = 0.0f;
        player->currentPlayerState = STATE_DASHING;
    }
    if (player->isDashing)
    {
        player->dashTimer -= deltaTime;
        if (player->dashTimer <= 0.0f)
        {
            player->isDashing = false;
            player->velocityX *= 0.3f;
        }
        player->velocityY += GRAVITY * deltaTime * 0.2f;
        player->animationTimer += deltaTime;
        player->x += player->velocityX * deltaTime;
        PlayerResolveCollision(player, level, true);
        player->y += player->velocityY * deltaTime;
        PlayerResolveCollision(player, level, false);
        return;
    }
    else
    {
        if (player->dashCooldown > 0.0f)
        {
            player->dashCooldown -= deltaTime;

            if (player->dashCooldown < 0.0f)
            {
                player->dashCooldown = 0.0f;
            }
        }
    }
    /* Normal movement */
    float targetVelocityX = 0.0f;
    if (moveLeftKeyDown)
    {
        targetVelocityX = -PLAYER_SPEED;
        if (!player->isWallSliding)
        {
            player->isFacingRight = false;
        }
    }
    if (moveRightKeyDown)
    {
        targetVelocityX = PLAYER_SPEED;
        if (!player->isWallSliding)
        {
            player->isFacingRight = true;
        }
    }
    float acceleration;
    if (player->isOnGround)
    {
        acceleration = 25.0f;
    }
    else
    {
        acceleration = 12.0f;
    }
    player->velocityX += (targetVelocityX - player->velocityX) * acceleration * deltaTime;
    /* Wall sliding detection */
    player->isWallSliding = false;
    player->canWallJump = false;
    player->wallDirection = 0;
    if (!player->isOnGround && player->velocityY > 0)
    {
        int tileXLeft = (int)(player->x / TILE_SIZE);
        int tileXRight = (int)((player->x + player->width - 1.0f) / TILE_SIZE);
        int tileYTop = (int)(player->y / TILE_SIZE);
        int tileYBottom = (int)((player->y + player->height - 1.0f) / TILE_SIZE);
        bool isWallOnLeft = false;
        bool isWallOnRight = false;
        for (int tileY = tileYTop; tileY <= tileYBottom && tileY < level->height; tileY++)
        {
            if (tileY < 0)
            {
                continue;
            }
            if (tileXLeft >= 0 && tileXLeft < level->width && level->tiles[tileY * level->width + tileXLeft] != 0)
            {
                isWallOnLeft = true;
            }
            if (tileXRight >= 0 && tileXRight < level->width && level->tiles[tileY * level->width + tileXRight] != 0)
            {
                isWallOnRight = true;
            }
        }
        if (moveLeftKeyDown && isWallOnLeft)
        {
            player->isWallSliding = true;
            player->wallDirection = -1;
            player->isFacingRight = true;
        }
        else if (moveRightKeyDown && isWallOnRight)
        {
            player->isWallSliding = true;
            player->wallDirection = 1;
            player->isFacingRight = false;
        }
    }

    if (player->isWallSliding)
    {
        player->velocityY =
            fminf(player->velocityY, WALL_SLIDE_SPEED);

        player->wallSlideTimer += deltaTime;
        player->canWallJump = true;
    }
    else
    {
        player->wallSlideTimer = 0.0f;
    }

    /* Jumping */
    if (jumpBuffer > 0.0f && (player->isOnGround || coyoteTimer > 0.0f))
    {
        player->velocityY = -JUMP_FORCE;
        jumpBuffer = 0.0f;
        coyoteTimer = 0.0f;
        player->isOnGround = false;

        player->currentPlayerState = STATE_JUMPING;
    }
    /* Wall Jumping */
    if (jumpBuffer > 0.0f && player->canWallJump)
    {
        player->velocityY = -WALL_JUMP_FORCE_Y;

        player->velocityX =
            -player->wallDirection * WALL_JUMP_FORCE_X;

        player->isWallSliding = false;
        player->canWallJump = false;
        jumpBuffer = 0.0f;
        player->currentPlayerState = STATE_JUMPING;

        if (player->velocityX > 0.0f)
        {
            player->isFacingRight = true;
        }
        else
        {
            player->isFacingRight = false;
        }
    }
    /* Variable jump height */
    if (!jumpKeyDown && player->velocityY < -180.0f && !player->isWallSliding)
    {
        player->velocityY *= 0.88f;
    }
    /* Gravity */
    if (!player->isWallSliding)
    {
        player->velocityY += GRAVITY * deltaTime;
    }
    else
    {
        player->velocityY += GRAVITY * deltaTime * 0.3f;
    }
    if (player->velocityY > MAX_FALL_SPEED)
    {
        player->velocityY = MAX_FALL_SPEED;
    }
    /* Move and resolve collisions */
    player->x += player->velocityX * deltaTime;
    PlayerResolveCollision(player, level, true);
    player->y += player->velocityY * deltaTime;
    PlayerResolveCollision(player, level, false);
    /* Update state for animation */
    if (player->isDashing)
    {
        player->currentPlayerState = STATE_DASHING;
    }
    else if (player->isWallSliding)
    {
        player->currentPlayerState = STATE_WALL_SLIDING;
    }
    else if (!player->isOnGround)
    {
        if (player->velocityY < 0)
        {
            player->currentPlayerState = STATE_JUMPING;
        }
        else
        {
            player->currentPlayerState = STATE_FALLING;
        }
    }
    else if (fabsf(player->velocityX) > 10.0f)
    {
        player->currentPlayerState = STATE_RUNNING;
    }
    else
    {
        player->currentPlayerState = STATE_IDLE;
    }
    player->animationTimer += deltaTime;
}
void PlayerRender(const Player *player, SDL_Renderer *renderer, float cameraX, float cameraY, SDL_Texture *sheet)
{
    float drawX = player->x - cameraX - (SPRITE_WIDTH - player->width) / 2.0f;
    float drawY = player->y - cameraY - (SPRITE_HEIGHT - player->height);
    SDL_FRect destinationFRect = {drawX, drawY, SPRITE_WIDTH * 2.0f, SPRITE_HEIGHT * 2.0f};
    if (sheet)
    {
        int frameCol = 0;
        int frameRow = player->styleRow;
        float animSpeed = 8.0f;
        switch (player->currentPlayerState)
        {
        case STATE_IDLE:
            frameCol = ANIMATION_IDLE_1;
            break;
        case STATE_RUNNING:
            animSpeed = 12.0f;
            frameCol = ANIMATION_RUN_1 + ((int)(player->animationTimer * animSpeed) % 6);
            frameRow = player->styleRow + 1;
            break;
        case STATE_JUMPING:
            frameCol = ANIMATION_JUMP;
            break;
        case STATE_FALLING:
            frameCol = ANIMATION_JUMP;
            break;
        case STATE_WALL_SLIDING:
            frameCol = ANIMATION_TURN;
            break;
        case STATE_DASHING:
            frameCol = ANIMATION_SLIDE_1;
            frameRow = player->styleRow + 1;
            break;
        case STATE_SLIDING:
            frameCol = ANIMATION_SLIDE_1;
            frameRow = player->styleRow + 1;
            break;
        }
        SDL_FRect sourceFRect = {
            frameCol * SPRITE_WIDTH,
            frameRow * SPRITE_HEIGHT,
            SPRITE_WIDTH,
            SPRITE_HEIGHT};
        SDL_FlipMode flip;

        if (player->isFacingRight)
        {
            flip = SDL_FLIP_NONE;
        }
        else
        {
            flip = SDL_FLIP_HORIZONTAL;
        }
        /* Dash Trail Effect */
        if (player->isDashing)
        {
            SDL_SetTextureColorMod(sheet, 255, 255, 255);
            SDL_FRect trailDestinationFRect = destinationFRect;
            if (player->isFacingRight)
            {
                trailDestinationFRect.x -= 10.0f;
            }
            else
            {
                trailDestinationFRect.x += 10.0f;
            }
            SDL_SetTextureAlphaMod(sheet, 128);
            SDL_RenderTextureRotated(renderer, sheet, &sourceFRect, &trailDestinationFRect, 0.0, NULL, flip);
            SDL_SetTextureAlphaMod(sheet, 255);
        }
        SDL_RenderTextureRotated(renderer, sheet, &sourceFRect, &destinationFRect, 0.0, NULL, flip);
    }
    else
    {
        SDL_FRect renderRect = {drawX, drawY, SPRITE_WIDTH * 2.0f, SPRITE_HEIGHT * 2.0f};
        switch (player->currentPlayerState)
        {
        case STATE_IDLE:
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            break;
        case STATE_RUNNING:
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            break;
        case STATE_JUMPING:
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            break;
        case STATE_FALLING:
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            break;
        case STATE_WALL_SLIDING:
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            break;
        case STATE_DASHING:
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            break;
        case STATE_SLIDING:
            SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
            break;
        }
        SDL_RenderFillRectF(renderer, &renderRect);
    }
}