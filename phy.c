#include "phy.h"
#include <math.h>  // For fabs
#include "sandbox.h"

#define double_jump_velocity -8.0f;
#define jump_velocity -10.0f // Jump velocity
#define max_jumptime 0.425f // Maximum time for jump hold
#define COYOTE_TIME 0.1f         // Allow jump 100ms after leaving ground
#define JUMP_BUFFER_TIME 0.1f    // Allow jump 100ms before landing
#define DASH_DURATION 0.2f
#define DASH_SPEED 15.0f
#define DASH_COOLDOWN 0.4f
#define KNOCKBACK_FORCE_X 13.0f   // Horizontal push speed
#define KNOCKBACK_FORCE_Y -7.0f  // Upward bounce (negative = up)
#define IFRAME_TIME 0.5f  // seconds
#define SLASH_DURATION 0.15f    // Increased from 0.2f to 0.3f
#define SLASH_WIDTH_MULT 1.8f  // Slash width multiplier
#define SLASH_HEIGHT_DIV 3.0f  // Slash height divider
#define ENEMY_DAMAGE_COOLDOWN 0.2f  // Add this with other #defines

float playerInvincibleTime = 0.1f;

static Camera2D gameCamera = { 0 };  // Static camera instance

Camera2D* getGameCamera(void) {
    return &gameCamera;
}

void handlePlayerCollisionDamage(Player *player, Entity *enemyEntity, int damage) {
    if (CheckCollisionRecs(player->base.hitbox, enemyEntity->hitbox) && player->iFrames <= 0.0f && enemyEntity->isAlive) {
        player->health -= damage;
        player->iFrames = IFRAME_TIME;

        // Knockback
        float knockDir = (player->base.hitbox.x < enemyEntity->hitbox.x) ? -1.0f : 1.0f;
        player->base.velocity.x = knockDir * KNOCKBACK_FORCE_X;
        player->base.velocity.y = KNOCKBACK_FORCE_Y;

        TraceLog(LOG_INFO, "Player hit! Health: %d", player->health);
    }
}
void initializePlayer(Player *player, float screenWidth, float screenHeight) {
    // Base entity properties
    player->base.hitbox = (Rectangle){ 
        screenWidth / 2.0f - 25, 
        screenHeight / 2.0f - 25, 
        35, 
        60 
    };
    player->base.velocity = (Vector2){ 0, 0 };
    player->base.onGround = false;

    // Health system
    player->maxHealth = 3;
    player->health = player->maxHealth;
    player->isAlive = true;
    player->deathTimer = 0.0f;
    player->iFrames = 0.0f;

    // Movement system
    player->isJumping = false;
    player->jumpTime = 0.0f;
    player->coyoteTimer = 0.0f;
    player->jumpBufferTimer = 0.0f;
    player->isDashing = false;
    player->dashTime = 0.0f;
    player->dashCooldown = 0.0f;
    player->dashDirection = 1;
    player->knkbackTime = 0.0f;

    // Combat system
    player->isSlashing = false;
    player->slashTimer = 0.0f;
    player->slashDuration = SLASH_DURATION;  // Use the constant instead of hardcoded value
    player->slashHitbox = (Rectangle){ 0, 0, 40, 60 };
    player->facingDirection = 1;  // Start facing right
    player->hasDoubleJump = true; 
}

void applyGravity(Entity *entity, float gravity, float gravityscale) {
    entity->velocity.y += gravity * gravityscale; // Apply gravity
}
void handleJump(Player *entity) {
    float dt = GetFrameTime();

    // --- Update timers ---
    if (entity->base.onGround) {
        entity->coyoteTimer = COYOTE_TIME;
        entity->hasDoubleJump = true; 
    } else {
        entity->coyoteTimer -= dt;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        entity->jumpBufferTimer = JUMP_BUFFER_TIME;
    } else {
        entity->jumpBufferTimer -= dt;
    }

    // --- Normal jump (ground + coyote) ---
    if (entity->jumpBufferTimer > 0 && entity->coyoteTimer > 0) {
        entity->isJumping = true;
        entity->jumpTime = 0.0f;
        entity->base.velocity.y = jump_velocity;
        entity->base.onGround = false;

        entity->jumpBufferTimer = 0.0f;
        entity->coyoteTimer = 0.0f;
    }

    // --- Hollow Knight style double jump ---
    else if (IsKeyPressed(KEY_SPACE) && !entity->isJumping&& entity->hasDoubleJump) {
        // snap upward regardless of falling or rising
        entity->isJumping = true;
        entity->jumpTime = 0.0f;
        entity->base.velocity.y = double_jump_velocity;

        entity->hasDoubleJump = false;   // consume double jump
    }

    // --- Variable jump height (only affects upward part of jump) ---
    if (entity->isJumping) {
        entity->jumpTime += dt;
        if (!IsKeyDown(KEY_SPACE) || entity->jumpTime >= max_jumptime) {
            entity->isJumping = false;
        }
    }

    // --- Gravity ---
    float gravityScale = (entity->isJumping && entity->base.velocity.y < 0)
                         ? 0.3f   // feathered rising
                         : 1.5f;  // heavy falling

    applyGravity(&entity->base, CONST_GRAVITY, gravityScale);
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
    float dt = GetFrameTime();

    // Check for death first
    if (enemy->health <= 0) {
        enemy->deathTimer += dt;
        enemy->base.isAlive = false;  // Mark enemy as dead
        return;
    }

    // Update damage cooldown first
    if (enemy->damageCooldown > 0) {
        enemy->damageCooldown -= dt;
        if (enemy->damageCooldown < 0) enemy->damageCooldown = 0;
    }
    
    // Update existing projectiles
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (enemy->projectiles[i].active) {
            // Update projectile position
            enemy->projectiles[i].hitbox.x += enemy->projectiles[i].velocity.x * dt * 400.0f;
            enemy->projectiles[i].hitbox.y += enemy->projectiles[i].velocity.y * dt * 400.0f;
            
            // Check parried projectile hits
            if (enemy->projectiles[i].isParried && 
                CheckCollisionRecs(enemy->projectiles[i].hitbox, enemy->base.hitbox)) {
                // Damage enemy when hit by parried projectile
                if (enemy->damageCooldown <= 0) {
                    enemy->health -= 2;  // Parried projectile damage
                    enemy->damageCooldown = ENEMY_DAMAGE_COOLDOWN;
                    TraceLog(LOG_INFO, "Enemy hit by parried projectile! Health: %d", enemy->health);
                }
                
                // Deactivate projectile and apply knockback
                enemy->projectiles[i].active = false;
                enemy->base.velocity.x = -KNOCKBACK_FORCE_X;
                enemy->base.velocity.y = KNOCKBACK_FORCE_Y;
                continue;
            }
            
            // Rest of the collision checks...
            // Check collision with platforms
            for (int j = 0; j < platformCount; j++) {
                if (CheckCollisionRecs(enemy->projectiles[i].hitbox, platforms[j])) {
                    enemy->projectiles[i].active = false;
                    break;
                }
            }
            
            // Check collision with player
            if (CheckCollisionRecs(enemy->projectiles[i].hitbox, player->base.hitbox) && player->isAlive) {
                if (player->iFrames <= 0.0f) {
                    player->health--;
                    player->iFrames = IFRAME_TIME;
                    
                    // Add knockback
                    float knockDir = (player->base.hitbox.x < enemy->projectiles[i].hitbox.x) ? -1.0f : 1.0f;
                    player->base.velocity.x = knockDir * KNOCKBACK_FORCE_X;
                    player->base.velocity.y = KNOCKBACK_FORCE_Y;
                }
                enemy->projectiles[i].active = false;
            }
            
            // Deactivate if off screen (optional)
            if (enemy->projectiles[i].hitbox.x < -1000 || enemy->projectiles[i].hitbox.x > 10000) {
                enemy->projectiles[i].active = false;
            }
        }
    }
    
    // Attack cooldown and charging logic
    if (enemy->attackCooldown > 0) {
        enemy->attackCooldown -= dt;
    }
    
    if (enemy->attackCooldown <= 0 && !enemy->isCharging) {
        enemy->isCharging = true;
        enemy->attackWindup = 1.0f;
        enemy->base.velocity.x = 0;
    }
    
    if (enemy->isCharging) {
        enemy->attackWindup -= dt;
        if (enemy->attackWindup <= 0) {
            // Create new projectile
            for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
                if (!enemy->projectiles[i].active) {
                    // Calculate direction vector
                    Vector2 direction = {
                        player->base.hitbox.x - enemy->base.hitbox.x,
                        player->base.hitbox.y - enemy->base.hitbox.y
                    };
                    
                    // Normalize direction vector
                    float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
                    direction.x /= length;
                    direction.y /= length;
                    
                    // Create projectile
                    enemy->projectiles[i].active = true;
                    enemy->projectiles[i].isParried = false;  // Initialize as not parried
                    enemy->projectiles[i].hitbox = (Rectangle){
                        enemy->base.hitbox.x + enemy->base.hitbox.width/2,
                        enemy->base.hitbox.y + enemy->base.hitbox.height/2,
                        15, 15  // Projectile size
                    };
                    enemy->projectiles[i].velocity = direction;
                    break;
                }
            }
            
            enemy->isCharging = false;
            enemy->attackCooldown = 2.0f;
        }
        return;
    }

    if (!player->isAlive) {
        enemy->base.velocity.x = 0;
        applyGravity(&enemy->base, 0.5f, 1);
        updateEntity(&enemy->base, platforms, platformCount, 0.0f);
        return;
    }

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

void updatePlayer(Player *player, Enemy *enemy, Rectangle *platforms) {
    float dt = GetFrameTime();

    // Check for death first - before any other updates
    if (player->health <= 0 && player->isAlive) {
        player->isAlive = false;
        player->deathTimer = 0.0f;
        TraceLog(LOG_INFO, "Player died - health reached 0");
        return;
    }

    // If already dead, just update death timer
    if (!player->isAlive) {
        player->deathTimer += dt;
        return;
    }

    // Update timers
    if (player->iFrames > 0.0f) player->iFrames -= dt;
    if (player->knkbackTime > 0.0f) player->knkbackTime -= dt;

    // Movement & physics
    handleDash(player);
    handleJump(player);
    handleSlash(player, enemy);
    updateEntity(&player->base, platforms, SMALL_PLATFORM_COUNT + 1, player->knkbackTime);  // +1 for floor

    // Update facing direction based on movement
    if (!player->isDashing && player->knkbackTime <= 0.0f) {
        if (IsKeyDown(KEY_D)) player->facingDirection = 1;
        if (IsKeyDown(KEY_A)) player->facingDirection = -1;
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

void handleSlash(Player *player, Enemy *enemy) {
    float dt = GetFrameTime();

    // Handle starting new slash
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !player->isSlashing) {
        player->isSlashing = true;
        player->slashTimer = 0.0f;
        player->slashDuration = SLASH_DURATION;
        
        float slashWidth = player->base.hitbox.width * SLASH_WIDTH_MULT;
        float slashHeight = player->base.hitbox.height / SLASH_HEIGHT_DIV;
        
        // Store the slash position when created - won't update with player movement
        player->slashHitbox = (Rectangle){
            player->facingDirection == 1 ? 
                player->base.hitbox.x + player->base.hitbox.width : 
                player->base.hitbox.x - slashWidth,
            player->base.hitbox.y + (player->base.hitbox.height - slashHeight) / 2,
            slashWidth,
            slashHeight
        };
    }

    // Update existing slash
    if (player->isSlashing) {
        player->slashTimer += dt;
        
        if (enemy->damageCooldown <= 0) {
            // Check for projectile parries first
            for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
                if (enemy->projectiles[i].active && !enemy->projectiles[i].isParried) {
                    if (CheckCollisionRecs(player->slashHitbox, enemy->projectiles[i].hitbox)) {
                        // Only reflect projectile, no damage on parry
                        enemy->projectiles[i].velocity.x *= -2.0f;
                        enemy->projectiles[i].velocity.y *= -2.0f;
                        enemy->projectiles[i].isParried = true;
                        
                        TraceLog(LOG_INFO, "Projectile parried!");
                        return;  // Exit after parry
                    }
                }
            }
            
            // Regular slash check
            if (CheckCollisionRecs(player->slashHitbox, enemy->base.hitbox)) {
                enemy->health--;
                enemy->damageCooldown = ENEMY_DAMAGE_COOLDOWN;
                
                // Knockback
                float knockDir = (enemy->base.hitbox.x < player->base.hitbox.x) ? -1.0f : 1.0f;
                enemy->base.velocity.x = knockDir * KNOCKBACK_FORCE_X;
                enemy->base.velocity.y = KNOCKBACK_FORCE_Y;
                
                TraceLog(LOG_INFO, "Enemy slashed! Health: %d", enemy->health);
            }
        }

        if (player->slashTimer >= player->slashDuration) {
            player->isSlashing = false;
        }
    }
}

