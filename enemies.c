#include "phy.h"
#include "sandbox.h"
#include "enemies.h"
#include <math.h>  
#include "fonts.h"



#define chaser_walk_speed 3.0f
#define chaser_run_speed 6.5f   
#define vision_range 400.0f


extern bool gatesOpen; // Global flag for gate state


void Chasers(chaser *enemy, Player *player, Rectangle *platforms, int platformCount){
    float dt = GetFrameTime();
    // Draw enemy 
    DrawRectangleRec(enemy->base.hitbox, ORANGE);
    // --- Patrol or chase ---
    if (enemy->isChasing) {
        enemy->base.velocity.x = (player->base.hitbox.x > enemy->base.hitbox.x) ? chaser_run_speed : -chaser_run_speed;
    } else {
        enemy->base.velocity.x = chaser_walk_speed * enemy->patrolDirection;
        enemy->base.hitbox.x += enemy->base.velocity.x * dt;

        // Reverse direction if exceeding patrol distance
        if (enemy->base.hitbox.x > enemy->spawnPos.x + enemy->patrolDistance)
            enemy->patrolDirection = -1;
        if (enemy->base.hitbox.x < enemy->spawnPos.x - enemy->patrolDistance)
            enemy->patrolDirection = 1;
    }

    // --- Gravity ---
    applyGravity(&enemy->base, CONST_GRAVITY, 1.0f);
    updateEntity(&enemy->base, platforms, platformCount, 0.0f);


    // --- Check for player in vision ---
    Rectangle vision = {
        (enemy->patrolDirection == 1) ? enemy->base.hitbox.x + enemy->base.hitbox.width : enemy->base.hitbox.x - vision_range,
        enemy->base.hitbox.y,
        vision_range,
        enemy->base.hitbox.height
    };
    if (CheckCollisionRecs(player->base.hitbox, vision)) {
        enemy->isChasing = true;
    } else if (fabs(player->base.hitbox.x - enemy->base.hitbox.x) > vision_range) {
        enemy->isChasing = false;
    }
}

void Boss1(boss1 *enemy, Player *player, Rectangle *platforms, int platformCount) {
    float dt = GetFrameTime();

    // If gates are still open, boss is idle (don’t update logic, just draw faintly)
    if (gatesOpen) {
        DrawRectangleRec(enemy->base.hitbox, Fade(GRAY, 0.3f));
        return;
    }

    // --- Dead boss fade out ---
    if (!enemy->base.isAlive) {
        float alpha = 1.0f - (enemy->deathTimer / 2.0f);
        if (alpha < 0) alpha = 0;
        DrawRectangleRec(enemy->base.hitbox, Fade(MAROON, alpha));
        enemy->deathTimer += dt;
        return;
    }

    // --- If boss is NOT awake yet ---
    if (!enemy->isAwake) {
        // Draw him idle
        DrawRectangleRec(enemy->base.hitbox, Fade(PURPLE, 0.5f));

        // Wake up if slashed
        if (player->isSlashing && CheckCollisionRecs(player->slashHitbox, enemy->base.hitbox)) {
            enemy->health -= 1;      
            enemy->isAwake = true;   
            enemy->showName = true;
            enemy->nameTimer = 3.0f;   // show for 3 seconds
            if (enemy->health <= 0) {
                enemy->base.isAlive = false;
                enemy->deathTimer = 0.0f;
            }
        }

        return; // don’t run AI until awake
    }

    // -------------------------------
    // From here on: boss is awake
    // -------------------------------

    // Boss size
    enemy->base.hitbox.width = 120;
    enemy->base.hitbox.height = 140;

    // Timers (static so they persist)
    static float chargeTimer = 0.0f;
    static float cooldownTimer = 0.0f;
    static float windupTimer = 0.0f;

    float chargeDuration = 1.5f;
    float chargeCooldown = 3.0f;
    float windupDuration = 0.7f;
    float walkSpeed = 2.0f;
    float chargeSpeed = 6.0f;

    // Gravity + physics
    applyGravity(&enemy->base, CONST_GRAVITY, 1.0f);
    updateEntity(&enemy->base, platforms, platformCount, 0.0f);

    // --- AI state machine ---
    if (enemy->isCharging) {
        chargeTimer += dt;
        float dx = player->base.hitbox.x - enemy->base.hitbox.x;
        enemy->base.velocity.x = (dx > 0) ? chargeSpeed : -chargeSpeed;
        enemy->base.hitbox.x += enemy->base.velocity.x;

        if (chargeTimer >= chargeDuration) {
            enemy->isCharging = false;
            chargeTimer = 0.0f;
            cooldownTimer = chargeCooldown;
        }
    }
    else if (windupTimer > 0) {
        windupTimer -= dt;
        if (windupTimer <= 0) {
            enemy->isCharging = true;
        }
    }
    else {
        enemy->base.velocity.x = 0;

        if (cooldownTimer > 0) {
            cooldownTimer -= dt;
            // chase slowly
            float dx = player->base.hitbox.x - enemy->base.hitbox.x;
            if (fabs(dx) > 10) {
                enemy->base.velocity.x = (dx > 0) ? walkSpeed : -walkSpeed;
                enemy->base.hitbox.x += enemy->base.velocity.x;
            }
        } else {
            // Ready for new charge
            float dx = player->base.hitbox.x - enemy->base.hitbox.x;
            if (fabs(dx) < 500) {
                windupTimer = windupDuration;
            } else if (fabs(dx) > 10) {
                // still chase slowly
                enemy->base.velocity.x = (dx > 0) ? walkSpeed : -walkSpeed;
                enemy->base.hitbox.x += enemy->base.velocity.x;
            }
        }
    }

    // --- Draw boss ---
    if (enemy->isCharging) {
        DrawRectangleRec(enemy->base.hitbox, DARKPURPLE);
        DrawRectangleLinesEx(enemy->base.hitbox, 4, RED);
    }
    else if (windupTimer > 0) {
        float pulse = 0.5f + 0.5f * sinf(GetTime() * 15.0f);
        DrawRectangleRec(enemy->base.hitbox, Fade(RED, 0.5f + 0.5f * pulse));
    }
    else {
        DrawRectangleRec(enemy->base.hitbox, PURPLE);
    }

    // --- Player damage when colliding with boss ---
    if (CheckCollisionRecs(player->base.hitbox, enemy->base.hitbox)) {
        handlePlayerCollisionDamage(player, &enemy->base, 2);
    }

    // --- Boss taking damage from melee slash ---
    if (player->isSlashing && CheckCollisionRecs(player->slashHitbox, enemy->base.hitbox)) {
        enemy->health -= 1;
        if (enemy->health <= 0) {
            enemy->base.isAlive = false;
            enemy->deathTimer = 0.0f;
        }
    }

// --- Boss Name popup ---
if (enemy->showName) {
    enemy->nameTimer -= dt;
    if (enemy->nameTimer <= 0) {
        enemy->showName = false;
    } else {
        // Fade during the last second
        float alpha = (enemy->nameTimer > 1.0f) ? 1.0f : enemy->nameTimer / 1.0f;
        Color nameColor = Fade(MAROON, alpha);

        const char *bossName = "THE BRUTE";
        int fontSize = 40;
        int padding = 20;

        Vector2 size = MeasureTextEx(msgFont, bossName, fontSize, 2);

        DrawTextEx(
            msgFont,
            bossName,
            (Vector2){ GetScreenWidth() - size.x - padding, GetScreenHeight() - fontSize - padding },
            fontSize,
            2,
            nameColor
        );
    }
}
}




