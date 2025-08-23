#include "phy.h"
#include <math.h>  // For fabs
#include "sandbox.h"
#include <stdio.h>

#define double_jump_velocity -8.0f;
#define jump_velocity -10.0f // Jump velocity
#define max_jumptime 0.425f // Maximum time for jump hold
#define COYOTE_TIME 0.1f         // Allow jump 100ms after leaving ground
#define JUMP_BUFFER_TIME 0.1f    // Allow jump 100ms before landing
#define DASH_DURATION 0.2f
#define DASH_SPEED 15.0f
#define DASH_COOLDOWN 0.4f
#define KNOCKBACK_FORCE_X 0.0f   // Horizontal push speed
#define KNOCKBACK_FORCE_Y 0.0f // Upward bounce (negative = up)
#define IFRAME_TIME 1.0f  // seconds
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
    if (CheckCollisionRecs(player->base.hitbox, enemyEntity->hitbox) && player->base.damageCooldown <= 0.0f && enemyEntity->isAlive) {
        player->base.health -= damage;
        player->base.damageCooldown = IFRAME_TIME;

        // Knockback
        float knockDir = (player->base.hitbox.x < enemyEntity->hitbox.x) ? -1.0f : 1.0f;
        player->base.velocity.x = knockDir * KNOCKBACK_FORCE_X;
        player->base.velocity.y = KNOCKBACK_FORCE_Y;

        TraceLog(LOG_INFO, "Player hit! Health: %d", player->base.health);
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
    player->maxHealth = 5;
    player->base.health = player->maxHealth;
    player->isAlive = true;
    player->deathTimer = 0.0f;
    player->base.damageCooldown = 0.0f;
    player->maxFlaskCharges = 3;  
    player->flaskCharges = 0;
    player->maxMana = 100.0f;
    player->mana = 0.0f;
    player->isHealing = false;
    player->healTimer = 0.0f;
    player->healDuration = 1.5f; 

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
    player->respawnPoint = (Vector2){ screenWidth / 2.0f - 25,screenHeight / 2.0f - 25,  }; // example coordinates

    // Combat system
    player->isSlashing = false;
    player->slashTimer = 0.0f;
    player->slashDuration = SLASH_DURATION;  // Use the constant instead of hardcoded value
    player->slashHitbox = (Rectangle){ 0, 0, 40, 60 };
    player->facingDirection = 1;  // Start facing right
    player->hasDoubleJump = true; 
    player->slashCooldown = 0.0f; 
    player->slashAnchorFacing = player->facingDirection; 
    bool slashFlippedMid = false;
    player->recoilTime = 0.0f;
    player->recoilDuration = 0.1f;   // recoil lasts ~100ms
    player->recoilStrength = 30.9f;   // adjust strength
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

    // --- Check for death ---
    if (enemy->base.health <= 0) {
        enemy->deathTimer += dt;
        enemy->base.isAlive = false;
        return;
    }

    // --- Damage cooldown ---
    if (enemy->base.damageCooldown > 0) {
        enemy->base.damageCooldown -= dt;
        if (enemy->base.damageCooldown < 0) enemy->base.damageCooldown = 0;
    }

    // --- Projectiles update (including parry) ---
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        EnemyProjectile *p = &enemy->projectiles[i];
        if (!p->active) continue;

        // Move projectile
        p->hitbox.x += p->velocity.x * dt * 650.0f;
        p->hitbox.y += p->velocity.y * dt * 650.0f;

        // --- Check for player parry ---
        if (!p->isParried && player->isSlashing && CheckCollisionRecs(player->slashHitbox, p->hitbox)) {
            p->isParried = true;
            p->velocity.x = -p->velocity.x * 2.0f;
            p->velocity.y = -p->velocity.y * 2.0f;
            TraceLog(LOG_INFO, "Projectile parried!");
        }

        // --- Parried projectile hits enemy ---
        if (p->isParried && CheckCollisionRecs(p->hitbox, enemy->base.hitbox)) {
            if (enemy->base.damageCooldown <= 0) {
                enemy->base.health -= 3; // Parried projectile damage
                enemy->base.damageCooldown = ENEMY_DAMAGE_COOLDOWN;
                TraceLog(LOG_INFO, "Enemy hit by parried projectile! Health: %d", enemy->base.health);
            }
            p->active = false;
            continue;
        }

        // --- Normal projectile hits player ---
        if (!p->isParried && CheckCollisionRecs(p->hitbox, player->base.hitbox) && player->isAlive) {
            if (player->base.damageCooldown <= 0.0f) {
                player->base.health--;
                player->base.damageCooldown = IFRAME_TIME;
                float knockDir = (player->base.hitbox.x < p->hitbox.x) ? -1.0f : 1.0f;
                player->base.velocity.x = knockDir * KNOCKBACK_FORCE_X;
                player->base.velocity.y = KNOCKBACK_FORCE_Y;
            }
            p->active = false;
        }

        // --- Collision with platforms ---
        for (int j = 0; j < platformCount; j++) {
            if (CheckCollisionRecs(p->hitbox, platforms[j])) {
                p->active = false;
                break;
            }
        }

        // --- Off-screen cleanup ---
        if (p->hitbox.x < -1000 || p->hitbox.x > 10000) {
            p->active = false;
        }
    }

    // --- Attack cooldown and projectile spawn ---
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
            // Spawn projectile toward player
            for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
                if (!enemy->projectiles[i].active) {
                    Vector2 dir = { player->base.hitbox.x - enemy->base.hitbox.x, player->base.hitbox.y - enemy->base.hitbox.y };
                    float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
                    dir.x /= len; dir.y /= len;

                    enemy->projectiles[i].active = true;
                    enemy->projectiles[i].isParried = false;
                    enemy->projectiles[i].hitbox = (Rectangle){ enemy->base.hitbox.x + enemy->base.hitbox.width/2,
                                                                enemy->base.hitbox.y + enemy->base.hitbox.height/2,
                                                                15, 15 };
                    enemy->projectiles[i].velocity = dir;
                    break;
                }
            }
            enemy->isCharging = false;
            enemy->attackCooldown = 2.0f;
        }

        applyGravity(&enemy->base, 0.5f, 1);
        updateEntity(&enemy->base, platforms, platformCount, 0.0f);
        return;
    }

    if (!player->isAlive) {
        enemy->base.velocity.x = 0;
        applyGravity(&enemy->base, 0.5f, 1);
        updateEntity(&enemy->base, platforms, platformCount, 0.0f);
        return;
    }

    // --- Horizontal AI movement ---
    float distanceX = player->base.hitbox.x - enemy->base.hitbox.x;
    if (fabs(distanceX) > chaseThreshold) {
        enemy->base.velocity.x = (distanceX > 0) ? chaseSpeed : -chaseSpeed;
    } else {
        enemy->base.velocity.x = 0;
    }

    applyGravity(&enemy->base, 0.5f, 1);
    updateEntity(&enemy->base, platforms, platformCount, 0.0f);

    // --- Vertical overlap correction ---
    if (CheckCollisionRecs(player->base.hitbox, enemy->base.hitbox)) {
        if (player->base.velocity.y > 0 &&
            player->base.hitbox.y + player->base.hitbox.height - enemy->base.hitbox.y < 10) {
            player->base.hitbox.y = enemy->base.hitbox.y - player->base.hitbox.height;
            player->base.velocity.y = 0;
            player->base.onGround = true;
        } else if (enemy->base.velocity.y > 0 &&
                   enemy->base.hitbox.y + enemy->base.hitbox.height - player->base.hitbox.y < 10) {
            enemy->base.hitbox.y = player->base.hitbox.y - enemy->base.hitbox.height;
            enemy->base.velocity.y = 0;
            enemy->base.onGround = true;
        }
    }
}


void updatePlayer(Player *player, Enemy *enemy, Rectangle *platforms) {
    float dt = GetFrameTime();

    // Death check
    if (player->base.health <= 0 && player->isAlive) {
        player->isAlive = false;
        player->deathTimer = 0.0f;
        TraceLog(LOG_INFO, "Player died - health reached 0");
        return;
    }

    // If player is dead, update death timer
if (!player->isAlive) {
    player->deathTimer += GetFrameTime();

    // Duration of death animation (e.g., 2 seconds)
    float deathDuration = 2.0f;

    if (player->deathTimer >= deathDuration) {
        // Respawn player after animation
        player->isAlive = true;
        player->base.health = player->maxHealth;
        player->mana = 0; // optional
        player->base.hitbox.x = player->respawnPoint.x;
        player->base.hitbox.y = player->respawnPoint.y;
        player->deathTimer = 0.0f;

        TraceLog(LOG_INFO, "Player respawned at checkpoint");
    }

    // Skip the rest of update while dead
    return;
}

    // Update timers
    if (player->base.damageCooldown > 0.0f) player->base.damageCooldown -= dt;
    if (player->knkbackTime > 0.0f) player->knkbackTime -= dt;
    if (player->slashCooldown > 0.0f) player->slashCooldown -= dt;

    // --- Handle time-based recoil ---
    if (player->recoilTime > 0.0f) {
        player->recoilTime -= dt;

        float t = player->recoilTime / player->recoilDuration;

        // apply decaying recoil every frame
        player->base.hitbox.x += player->recoilVelocityX * t;

        if (player->recoilTime <= 0.0f) {
            player->recoilVelocityX = 0;
        }
    }

    // Movement & combat
    handleDash(player);
    handleJump(player);
    handleSlash(player, (Entity*)&enemy->base); 

    

    updateEntity(&player->base, platforms, SMALL_PLATFORM_COUNT + 7, player->knkbackTime);

    // Update facing direction
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


// --- Slash ---
// --- Slash ---
void handleSlash(Player *player, Entity *target) {
    float dt = GetFrameTime();

    // --- Start a new slash ---
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        !player->isSlashing &&
        player->slashCooldown <= 0.0f) 
    {
        player->isSlashing = true;
        player->slashTimer = 0.0f;
        player->slashDuration = SLASH_DURATION;
        player->slashCooldown = 0.35f;

        // Decide slash direction
        if (IsKeyDown(KEY_W)) 
            player->slashDirection = SLASH_UP;
        else if (IsKeyDown(KEY_S) && !player->base.onGround) 
            player->slashDirection = SLASH_DOWN;
        else 
            player->slashDirection = SLASH_FORWARD;

        player->slashAnchorFacing = player->facingDirection;
        player->slashFlippedMid = false;

        float slashWidth  = player->base.hitbox.width * SLASH_WIDTH_MULT;
        float slashHeight = player->base.hitbox.height / SLASH_HEIGHT_DIV;

        switch (player->slashDirection) {
            case SLASH_FORWARD:
                player->slashHitbox = (Rectangle){
                    player->slashAnchorFacing == 1 ?
                        player->base.hitbox.x + player->base.hitbox.width :
                        player->base.hitbox.x - slashWidth,
                    player->base.hitbox.y + (player->base.hitbox.height - slashHeight) / 2,
                    slashWidth,
                    slashHeight
                };
                break;
            case SLASH_UP:
                player->slashHitbox = (Rectangle){
                    player->base.hitbox.x + (player->base.hitbox.width - slashHeight) / 2,
                    player->base.hitbox.y - slashWidth,
                    slashHeight,
                    slashWidth
                };
                break;
            case SLASH_DOWN:
                player->slashHitbox = (Rectangle){
                    player->base.hitbox.x + (player->base.hitbox.width - slashHeight) / 2,
                    player->base.hitbox.y + player->base.hitbox.height,
                    slashHeight,
                    slashWidth
                };
                break;
        }
    }

    // --- While slashing ---
    if (player->isSlashing) {
        player->slashTimer += dt;

        float slashWidth  = player->base.hitbox.width * SLASH_WIDTH_MULT;
        float slashHeight = player->base.hitbox.height / SLASH_HEIGHT_DIV;

        // Detect mid-slash flip
        if (!player->slashFlippedMid && player->facingDirection != player->slashAnchorFacing) {
            player->slashFlippedMid = true;
        }

        // Update horizontal position only if not flipped mid-slash
        if (player->slashDirection == SLASH_FORWARD && !player->slashFlippedMid) {
            player->slashHitbox.x = (player->slashAnchorFacing == 1) ?
                                     player->base.hitbox.x + player->base.hitbox.width :
                                     player->base.hitbox.x - slashWidth;
        }

        // Vertical slashes always follow player horizontally
        if (player->slashDirection == SLASH_UP || player->slashDirection == SLASH_DOWN) {
            player->slashHitbox.x = player->base.hitbox.x + (player->base.hitbox.width - slashHeight) / 2;
        }

        // Vertical positions
        if (player->slashDirection == SLASH_UP) {
            player->slashHitbox.y = player->base.hitbox.y - slashWidth;
        } else if (player->slashDirection == SLASH_DOWN) {
            player->slashHitbox.y = player->base.hitbox.y + player->base.hitbox.height;
        } else if (player->slashDirection == SLASH_FORWARD) {
            player->slashHitbox.y = player->base.hitbox.y + (player->base.hitbox.height - slashHeight) / 2;
        }

        // --- Collision with target entity ---
        if (target != NULL && target->isAlive && target->damageCooldown <= 0.0f) {
            if (CheckCollisionRecs(player->slashHitbox, target->hitbox)) {
                target->health--;
                target->damageCooldown = ENEMY_DAMAGE_COOLDOWN;
                if (player->mana < player->maxMana) {
                    player->mana += 12.0f; // gain 12 mana per hit
                    if (player->mana > player->maxMana) player->mana = player->maxMana;
                    TraceLog(LOG_INFO, "Mana gained! Current Mana: %.2f", player->mana);
                }

                // --- Apply knockback depending on slash type ---
                if (player->slashDirection == SLASH_FORWARD) {
                    float knockDir = (target->hitbox.x < player->base.hitbox.x) ? -1.0f : 1.0f;
                    target->velocity.x = knockDir * KNOCKBACK_FORCE_X;
                    target->velocity.y = KNOCKBACK_FORCE_Y; // slight lift
                } 
                else if (player->slashDirection == SLASH_UP) {
                    target->velocity.y = -KNOCKBACK_FORCE_Y;
                    target->velocity.x = 0;
                } 
                else if (player->slashDirection == SLASH_DOWN) {
                    target->velocity.y = KNOCKBACK_FORCE_Y;
                    target->velocity.x = 0;
                }

                // --- Player recoil (forward slashes only) ---
                if (player->slashDirection == SLASH_FORWARD) {
                    player->recoilTime = player->recoilDuration;
                    float recoilDir = (player->base.hitbox.x < target->hitbox.x) ? -1.0f : 1.0f;
                    player->recoilVelocityX = recoilDir * player->recoilStrength;
                }

                TraceLog(LOG_INFO, "Entity slashed! Health: %d", target->health);

                // --- POGO if down slash ---
                if (player->slashDirection == SLASH_DOWN) {
                    player->base.velocity.y = -12.0f;
                    player->base.onGround = false;
                    player->hasDoubleJump = true;
                    TraceLog(LOG_INFO, "POGO triggered!");
                }

                // Kill entity if health is gone
                if (target->health <= 0) {
                    target->isAlive = false;
                    TraceLog(LOG_INFO, "Entity defeated!");
                }
            }
        }

        // --- End slash after duration ---
        if (player->slashTimer >= player->slashDuration) {
            player->isSlashing = false;
        }
    }
}

void handleHealing(Player *player) {
    // Start healing if key is held and enough mana
    if (IsKeyDown(KEY_E) && player->mana >= 33.0f && player->base.health < player->maxHealth) {
        player->isHealing = true;
        player->healTimer += GetFrameTime();

        // If held for full duration
        if (player->healTimer >= player->healDuration) {
            player->base.health += 1;
            if (player->base.health > player->maxHealth) {
                player->base.health = player->maxHealth;
            }
            player->mana -= 33.0f;
            TraceLog(LOG_INFO, "Player healed! Current HP: %d | Mana left: %.2f", player->base.health, player->mana);

            player->healTimer = 0.0f;
            player->isHealing = false; // done healing
        }
    } else {
        // Cancel healing if key released early
        if (player->isHealing && !IsKeyDown(KEY_E)) {
            TraceLog(LOG_INFO, "Healing interrupted!");
        }
        player->healTimer = 0.0f;
        player->isHealing = false;
    }
}




void handleParry(Player *player, EnemyProjectile *projectiles, int projectileCount, Entity *boss, Enemy *enemies, int enemyCount) {
    if (!player->isSlashing) return; // only parry while slashing

    for (int i = 0; i < projectileCount; i++) {
        EnemyProjectile *p = &projectiles[i];
        if (!p->active) continue;

        // Check collision between player's slash and projectile
        if (CheckCollisionRecs(player->slashHitbox, p->hitbox)) {
            // Mark as parried
            p->isParried = true;

            // Reverse velocity at 2x speed
            p->velocity.x = -p->velocity.x * 2.0f;
            p->velocity.y = -p->velocity.y * 2.0f;

            TraceLog(LOG_INFO, "Projectile parried! Reflected at 2x speed.");
        }

        // If projectile is parried, check if it hits boss or enemies
        if (p->isParried) {
            if (boss && CheckCollisionRecs(p->hitbox, boss->hitbox)) {
                boss->health -= 3;
                p->active = false;
                TraceLog(LOG_INFO, "Boss hit by parry! Boss HP: %d", boss->health);
            }

            for (int j = 0; j < enemyCount; j++) {
                if (enemies[j].active && CheckCollisionRecs(p->hitbox, enemies[j].base.hitbox)) {
                    enemies[j].base.health -= 3;
                    p->active = false;
                    TraceLog(LOG_INFO, "Enemy hit by parry! Enemy HP: %d", enemies[j].base.health);
                }
            }
        }
    }
}

