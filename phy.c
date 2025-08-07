#include "sandbox.h"
#include "phy.h"

#define jump_velocity -10.0f // Jump velocity
#define max_jumptime 0.35f // Maximum time for jump hold
#define COYOTE_TIME 0.1f         // Allow jump 100ms after leaving ground
#define JUMP_BUFFER_TIME 0.1f    // Allow jump 100ms before landing
#define DASH_DURATION 0.2f
#define DASH_SPEED 15.0f
#define DASH_COOLDOWN 0.4f


void applyGravity(Entity *entity, float gravity){
    if(!entity->onGround) { 
        if(entity->isJumping) {
        entity->velocity.y += gravity * 0.3f; // lighter gravity when jumping
        }else{
            entity->velocity.y += gravity*1.5f; // normal gravity
        }
    } 
}
void handleJump(Entity *entity) {
    float dt = GetFrameTime();

    // Update timers
    if (entity->onGround) {
        entity->coyoteTimer = COYOTE_TIME;  // Reset when grounded
    } else {
        entity->coyoteTimer -= dt;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        entity->jumpBufferTimer = JUMP_BUFFER_TIME;  // Store jump intent
    } else {
        entity->jumpBufferTimer -= dt;
    }

    // Perform jump if within valid window
    if (entity->jumpBufferTimer > 0 && entity->coyoteTimer > 0) {
        entity->isJumping = true;
        entity->jumpTime = 0.0f;
        entity->velocity.y = jump_velocity;
        entity->onGround = false;

        // Reset buffer so it doesn’t trigger multiple times
        entity->jumpBufferTimer = 0.0f;
        entity->coyoteTimer = 0.0f;
    }

    // Handle variable height
    if (entity->isJumping) {
        entity->jumpTime += dt;
        if (!IsKeyDown(KEY_SPACE) || entity->jumpTime >= max_jumptime) {
            entity->isJumping = false;
        }
    }
}

void handleDash(Entity *entity) {
    float dt = GetFrameTime();

    if(entity -> dashCooldown > 0.0f){
        entity->dashCooldown -= dt; // Cooldown for dash
    }
    if(IsKeyPressed(KEY_LEFT_SHIFT) && entity->dashCooldown <= 0.0f && !entity->isDashing) {
        entity->isDashing = true;
        entity->dashTime = 0.0f; // Reset dash time
        entity->dashCooldown = DASH_COOLDOWN; // Reset cooldown


        if(IsKeyDown(KEY_D)) {
            entity->dashDirection = 1; // Right
        }else if(IsKeyDown(KEY_A)) {
            entity->dashDirection = -1; // Left
        }else entity->dashDirection = 1; // default to right if no key pressed
        

    }
    if(entity->isDashing) {
        entity->dashTime += dt;
        entity->velocity.x = entity->dashDirection * DASH_SPEED; // Apply dash speed   
        if(entity->dashTime >= DASH_DURATION) {
            entity->isDashing = false; // End dash
            entity->velocity.x = 0; // Reset velocity after dash
        }
    }
}


void updateEntity(Entity *player, Rectangle *floor, int platformCount){
    // Apply movement from velocity
    player->hitbox.y += player->velocity.y;
    player->onGround = false;

    for (int i = 0; i < platformCount; i++) {
        if (CheckCollisionRecs(player->hitbox, floor[i])) {
            // Snap to top of platform
            player->hitbox.y = floor[i].y - player->hitbox.height;
            player->velocity.y = 0;
            player->onGround = true;
        }
    }
}





