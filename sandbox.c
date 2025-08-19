#include "raylib.h"
#include "fonts.h"
#include "menu.h"
#include "sandbox.h"
#include "phy.h"
#include "gun.h"
#include "enemies.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define focusOffsetY 200  // Distance above player to center camera

bool debugMode = false;
int SMALL_PLATFORM_COUNT = 4;
float CONST_GRAVITY = 0.5f;


Rectangle *smallPlatforms;
Bullet bullets[MAX_BULLETS];

extern GameScreen currentScreen;

void sandBox() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float speed = 5;
    

    
    Rectangle floor = { 0, screenHeight - 100, screenWidth * 10, 150 };
    float floorTop = floor.y;
    float floorBottom = floor.y + floor.height;
    
    smallPlatforms = malloc(SMALL_PLATFORM_COUNT * sizeof(Rectangle));

    smallPlatforms[0] = (Rectangle){ screenWidth / 3, floorTop - 190, 100, 20 };
    smallPlatforms[1] = (Rectangle){ screenWidth / 2, floorTop - 290, 100, 20 };
    smallPlatforms[2] = (Rectangle){ screenWidth / 4, floorTop - 340, 100, 20 };
    smallPlatforms[3] = (Rectangle){ screenWidth / 1.5f, floorTop - 240, 100, 20 };
        

    Player player = {0};
    initializePlayer(&player, screenWidth, screenHeight);

    Enemy enemy = {0};
    enemy.base.hitbox = (Rectangle){ player.base.hitbox.x - 200, player.base.hitbox.y, 35, 60 };
    enemy.base.velocity = (Vector2){ 0, 0 };
    enemy.base.onGround = false;
    enemy.health = 5;
    enemy.deathTimer = 0.0f;
    enemy.attackCooldown = 0.0f;
    enemy.attackWindup = 0.0f;
    enemy.isCharging = false;
    enemy.damageCooldown = 0.0f;
    enemy.base.isAlive = true;

    chaser enemy1 = {0};
    enemy1.spawnPos.x = screenWidth / 2;
    enemy1.spawnPos.y = bossFloorTop ;
    enemy1.base.hitbox = (Rectangle){enemy1.spawnPos.x, enemy1.spawnPos.y, 35, 60};
    enemy1.base.velocity = (Vector2){0, 0};
    enemy1.isChasing = false;
    enemy1.patrolDirection = 1; 
    enemy1.patrolDistance = 200;


    // Use the global camera
    Camera2D *camera = getGameCamera();
    camera->offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera->zoom = 1.0f;

    InitBullets(bullets, MAX_BULLETS);

    while (currentScreen == SCREEN_SANDBOX && !WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) isPaused = !isPaused;

        if (IsKeyPressed(KEY_F1)) {
            debugMode = !debugMode;
            TraceLog(LOG_INFO, "Debug mode %s", debugMode ? "enabled" : "disabled");
        }
            if (!isPaused) {

        Rectangle platforms[1 + SMALL_PLATFORM_COUNT];
        platforms[0] = floor;
        for (int i = 0; i < SMALL_PLATFORM_COUNT; i++) {
        platforms[i + 1] = smallPlatforms[i];
        }

            // Movement
            if (!player.isDashing && player.knkbackTime <= 0.0f) {
                player.base.velocity.x = 0;
                if (IsKeyDown(KEY_D)) { player.base.velocity.x += speed; player.facingDirection = 1; }
                if (IsKeyDown(KEY_A)) { player.base.velocity.x -= speed; player.facingDirection = -1; }
            }

            // Shooting
            if (IsKeyPressed(KEY_M)) {
                Vector2 startPos = {
                    player.base.hitbox.x + (player.facingDirection == 1 ? player.base.hitbox.width : 0),
                    player.base.hitbox.y + player.base.hitbox.height / 2
                };
                ShootBullet(bullets, startPos, player.facingDirection);
            }

            player.base.hitbox.x += player.base.velocity.x;

            updatePlayer(&player, &enemy, platforms);
            updateEnemy(&enemy, &player, platforms, SMALL_PLATFORM_COUNT + 1, speed - 2, 5.0f);
            handlePlayerCollisionDamage(&player, &enemy.base, 1);

            if (player.knkbackTime > 0) player.knkbackTime -= GetFrameTime();

            UpdateBullets(bullets, MAX_BULLETS);

            float desiredY = player.base.hitbox.y + player.base.hitbox.height / 2 - focusOffsetY;
            float cameraMinY = floorBottom - screenHeight / 2.0f;
            if (desiredY > cameraMinY) desiredY = cameraMinY;

            camera->target = (Vector2){
                player.base.hitbox.x + player.base.hitbox.width / 2,
                desiredY
            };

        // Drawing - reorganized for proper order
        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(*camera);  // Use pointer dereference
        //Chasers(&enemy1, &player, platforms, SMALL_PLATFORM_COUNT + 1);
        // Background elements
        DrawRectangleRec(floor, BLACK);
        for (int i = 0; i < SMALL_PLATFORM_COUNT; i++) {
            DrawRectangleRec(smallPlatforms[i], PURPLE);
        }

            DrawBullets(bullets, MAX_BULLETS);

            if (!player.isAlive) {
                float alpha = 1.0f - (player.deathTimer / 2.0f);
                if (alpha < 0) alpha = 0;
                DrawRectangleRec(player.base.hitbox, Fade(RAYWHITE, alpha));
            } else {
                DrawRectangleRec(player.base.hitbox, RAYWHITE);
            }

            if (player.isSlashing) {
                float alpha = 1.0f - (player.slashTimer / player.slashDuration);
                DrawRectangleRec(player.slashHitbox, Fade(YELLOW, alpha));
            }

            if (!enemy.base.isAlive) {
                float alpha = 1.0f - (enemy.deathTimer / 1.0f);
                DrawRectangleRec(enemy.base.hitbox, Fade(RED, alpha));
            } else if (enemy.isCharging) {
                float pulseScale = 1.0f + sinf(enemy.attackWindup * 10) * 0.2f;
                Rectangle animatedRect = enemy.base.hitbox;
                animatedRect.width *= pulseScale;
                animatedRect.height *= pulseScale;
                animatedRect.x -= (animatedRect.width - enemy.base.hitbox.width) / 2;
                animatedRect.y -= (animatedRect.height - enemy.base.hitbox.height) / 2;
                DrawRectangleRec(animatedRect, RED);
                DrawRectangleRec(enemy.base.hitbox, ORANGE);
            } else {
                DrawRectangleRec(enemy.base.hitbox, RED);
            }

            for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
                if (enemy.projectiles[i].active) {
                    DrawRectangleRec(enemy.projectiles[i].hitbox, ORANGE);
                    if (debugMode) DrawRectangleLinesEx(enemy.projectiles[i].hitbox, 1, RED);
                }
            }
        }

        // Move the aim indicator to debug mode
        if (debugMode && enemy.isCharging) {
            Vector2 center = {
                enemy.base.hitbox.x + enemy.base.hitbox.width/2,
                enemy.base.hitbox.y + enemy.base.hitbox.height/2
            };
            Vector2 targetDirection = {
                player.base.hitbox.x - enemy.base.hitbox.x,
                player.base.hitbox.y - enemy.base.hitbox.y
            };
            float length = 30.0f;
            Vector2 endPoint = {
                center.x + (targetDirection.x/hypot(targetDirection.x, targetDirection.y)) * length,
                center.y + (targetDirection.y/hypot(targetDirection.x, targetDirection.y)) * length
            };
            DrawLineEx(center, endPoint, 3, YELLOW);
        }

        // Debug overlays
        if (debugMode) {
            DrawRectangleLines(player.base.hitbox.x, player.base.hitbox.y, player.base.hitbox.width, player.base.hitbox.height, BLUE);
            DrawRectangleLines(floor.x, floor.y, floor.width, floor.height, GREEN);
            DrawRectangleLines(enemy.base.hitbox.x, enemy.base.hitbox.y, enemy.base.hitbox.width, enemy.base.hitbox.height, ORANGE);
            for (int i = 0; i < SMALL_PLATFORM_COUNT; i++) {
            DrawRectangleLines(smallPlatforms[i].x, smallPlatforms[i].y, smallPlatforms[i].width, smallPlatforms[i].height, GREEN);
            }
            for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                DrawRectangleLinesEx(bullets[i].hitbox, 2, PURPLE); 
            }
                DrawLine(
                enemy1.base.hitbox.x + enemy1.base.hitbox.width / 2,
                enemy1.base.hitbox.y + enemy1.base.hitbox.height / 2,
                enemy1.base.hitbox.x + enemy1.base.hitbox.width / 2 + 400* enemy1.patrolDirection,
                enemy1.base.hitbox.y + enemy1.base.hitbox.height / 2,
                BLUE
            );
        }
        }
        if (!player.isAlive && player.deathTimer >= 2.0f) {
            currentScreen = SCREEN_MAIN_MENU; // After 2s fade
            TraceLog(LOG_INFO, "Player death animation done - returning to main menu");
        }
            }
        EndMode2D();
        if (isPaused) {
        ingameMenu();
        }
        // UI elements (after EndMode2D)
        for (int i = 0; i < player.maxHealth; i++) {
            Color heartColor = (i < player.health) ? RED : DARKGRAY;
            DrawRectangle(20 + i * 40, 20, 30, 30, heartColor);
        }

            EndDrawing();
        }
    }

    free(smallPlatforms);
}
