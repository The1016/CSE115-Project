#include "phy.h"
#include <math.h>  // For fabs
#include "sandbox.h"

#define jump_velocity -10.0f // Jump velocity
#define max_jumptime 0.35f // Maximum time for jump hold
#define COYOTE_TIME 0.1f         // Allow jump 100ms after leaving ground
#define JUMP_BUFFER_TIME 0.1f    // Allow jump 100ms before landing
#define DASH_DURATION 0.2f
#define DASH_SPEED 15.0f
#define DASH_COOLDOWN 0.4f
#define KNOCKBACK_FORCE_X 13.0f   // Horizontal push speed
#define KNOCKBACK_FORCE_Y -7.0f  // Upward bounce (negative = up)
#define IFRAME_TIME 0.5f  // seconds

float playerInvincibleTime = 0.1f;


void applyGravity(Entity *entity, float gravity, float gravityscale) {
    entity->velocity.y += gravity * gravityscale; // Apply gravity
}
void handleJump(Player *entity) {
    float dt = GetFrameTime();

    // Update timers
    if (entity->base.onGround) {
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
        entity->base.velocity.y = jump_velocity;
        entity->base.onGround = false;

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

    float gravityScale = (entity->isJumping && entity->base.velocity.y < 0)
                         ? 0.3f
                         : 1.5f;

    applyGravity( &entity->base, CONST_GRAVITY, gravityScale);
}

void handleDash(Player *entity) {
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
        entity->base.velocity.x = entity->dashDirection * DASH_SPEED; // Apply dash speed   
        if(entity->dashTime >= DASH_DURATION) {
            entity->isDashing = false; // End dash
            entity->base.velocity.x = 0; // Reset velocity after dash
        }
    }
}


void updateEnemy(Enemy *enemy, Player *player, Rectangle *platforms, int platformCount, float chaseSpeed, float chaseThreshold) {
    float distanceX = player->base.hitbox.x - enemy->base.hitbox.x;

    // Horizontal movement toward player
    if (fabs(distanceX) > chaseThreshold) {
        enemy->base.velocity.x = (distanceX > 0) ? chaseSpeed : -chaseSpeed;
    } else {
        enemy->base.velocity.x = 0;
    }

    applyGravity(&enemy->base, 0.5f, 1);
    updateEntity(&enemy->base, platforms, platformCount, 0.0f);

    // Stop overlapping vertically
    if (CheckCollisionRecs(player->base.hitbox, enemy->base.hitbox)) {
    // Player lands on top of enemy
    if (player->base.velocity.y > 0 &&
        player->base.hitbox.y + player->base.hitbox.height - enemy->base.hitbox.y < 10) {
        
        player->base.hitbox.y = enemy->base.hitbox.y - player->base.hitbox.height;
        player->base.velocity.y = 0;
        player->base.onGround = true;
    }
    // Enemy lands on top of player
    else if (enemy->base.velocity.y > 0 &&
             enemy->base.hitbox.y + enemy->base.hitbox.height - player->base.hitbox.y < 10) {
        
        enemy->base.hitbox.y = player->base.hitbox.y - enemy->base.hitbox.height;
        enemy->base.velocity.y = 0;
        enemy->base.onGround = true;
    }
}
}

void updatePlayer(Player *player, Enemy *enemy) {
    float dt = GetFrameTime();

    // decrement timers
    if (player->iFrames > 0.0f) player->iFrames -= dt;
    if (player->knkbackTime  > 0.0f) player->knkbackTime  -= dt;

    // detect hit (only if not invincible)
    if (player->iFrames <= 0.0f && CheckCollisionRecs(player->base.hitbox, enemy->base.hitbox)) {
        // knock direction: away from enemy
        float knockDir = (player->base.hitbox.x < enemy->base.hitbox.x) ? -1.0f : 1.0f;

        // apply knockback velocities
        player->base.velocity.x = knockDir * 9.0f;  // tune horizontal KB strength
        player->base.velocity.y = -9.0f;            // tune vertical bounce

        // timers
        player->iFrames = 1.0f;  // 1s of i-frames (tweak)
        player->knkbackTime  = 0.18f; // how long horizontal KB persists (tweak)

        // make sure player isn't considered grounded
        player->base.onGround = false;
    }
}

void resolveEntityCollision(Player *player, Enemy *enemy) {
    if (CheckCollisionRecs(player->base.hitbox, enemy->base.hitbox)) {
        // Only trigger if not invincible
        if (player->iFrames <= 0.0f) {
            player->health--;
            player->iFrames = IFRAME_TIME;      // invulnerability time
            player->knkbackTime = 0.2f;         // time during which control is disabled

            // Determine knockback direction (away from enemy)
            float knockDir = (player->base.hitbox.x < enemy->base.hitbox.x) ? -1.0f : 1.0f;

            // Apply knockback
            player->base.velocity.x = knockDir * KNOCKBACK_FORCE_X;
            player->base.velocity.y = KNOCKBACK_FORCE_Y;

            // Ensure player is considered airborne
            player->base.onGround = false;
        }
    }
}









void updateEntity(Entity *entity, Rectangle *platforms, int platformCount, float ignoreKnockbackTime) {
    // Only apply player-controlled horizontal movement if not in knockback
    if (ignoreKnockbackTime <= 0.0f) {
        entity->hitbox.x += entity->velocity.x;
    } else {
        // Knockback in progress, still apply velocity but no control override
        entity->hitbox.x += entity->velocity.x;
    }

    // --- Collision logic stays the same ---
    // (horizontal collision)
    for (int i = 0; i < platformCount; i++) {
        if (CheckCollisionRecs(entity->hitbox, platforms[i])) {
            if (entity->velocity.x > 0)
                entity->hitbox.x = platforms[i].x - entity->hitbox.width;
            else if (entity->velocity.x < 0)
                entity->hitbox.x = platforms[i].x + platforms[i].width;
            entity->velocity.x = 0;
        }
    }

    // (vertical movement)
    entity->hitbox.y += entity->velocity.y;
    entity->onGround = false;
    for (int i = 0; i < platformCount; i++) {
        if (CheckCollisionRecs(entity->hitbox, platforms[i])) {
            if (entity->velocity.y > 0) {
                entity->hitbox.y = platforms[i].y - entity->hitbox.height;
                entity->onGround = true;
            } else if (entity->velocity.y < 0) {
                entity->hitbox.y = platforms[i].y + platforms[i].height;
            }
            entity->velocity.y = 0;
        }
    }
}

