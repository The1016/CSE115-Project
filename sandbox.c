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
#include "zones.h"


#define focusOffsetY 200  // Distance above player to center camera

bool debugMode = false;
int SMALL_PLATFORM_COUNT = 4;
float CONST_GRAVITY = 0.5f;
bool gatesOpen = true; 


Rectangle *smallPlatforms;
Bullet bullets[MAX_BULLETS];

extern GameScreen currentScreen;  // Declare external variable

void sandBox() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float speed = 5;
    

    
    Rectangle floor = { 0, screenHeight - 100, screenWidth * 2, 150 };
    Rectangle bossFloor = { screenWidth * 2 +150, screenHeight - 100, screenWidth * 2, 150};
    float floorTop = floor.y;
    float floorBottom = floor.y + floor.height;
    float bossFloorTop = bossFloor.y;
    
    smallPlatforms = malloc(SMALL_PLATFORM_COUNT * sizeof(Rectangle));
    

    smallPlatforms[0] = (Rectangle){ screenWidth / 3, floorTop - 190, 100, 20 };
    smallPlatforms[1] = (Rectangle){ screenWidth / 2, floorTop - 290, 100, 20 };
    smallPlatforms[2] = (Rectangle){ screenWidth / 4, floorTop - 340, 100, 20 };
    smallPlatforms[3] = (Rectangle){ screenWidth / 1.5f, floorTop - 240, 100, 20 };
    
    
    Rectangle bossRoomL = { screenWidth * 2 + 250, bossFloorTop - 1000, 50, 850 };
    Rectangle bossRoomR =  { screenWidth*3 +190, bossFloorTop - 1000, 50, 850 };
    Rectangle Lgate = { screenWidth * 2 + 250, bossFloorTop-150 , 50, 150 };
    Rectangle Rgate = { screenWidth * 3 + 190, bossFloorTop-150 , 50, 150 };
    Rectangle celling = { screenWidth * 2 +250, screenHeight - 900, screenWidth * 2, 150 };


    Player player = {0};
    initializePlayer(&player, screenWidth, screenHeight);

    Enemy enemy = {0};
    enemy.base.hitbox = (Rectangle){ player.base.hitbox.x - 200, player.base.hitbox.y, 35, 60 };
    enemy.base.velocity = (Vector2){ 0, 0 };
    enemy.base.onGround = false;

    float floorBottom = floor->y + floor->height;
float cameraMinY = floorBottom - screenHeight / 2.0f;


    // Use the global camera
    Camera2D *camera = getGameCamera();
    camera->offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera->zoom = 1.0f;

    InitBullets(bullets, MAX_BULLETS);

    while (currentScreen == SCREEN_SANDBOX && !WindowShouldClose()) {
        // Bench interaction
        if (CheckCollisionRecs(player.base.hitbox, bench.hitbox) && IsKeyPressed(KEY_E)) {
            bench.isActive = true;
            TraceLog(LOG_INFO, "Checkpoint activated!");
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
        isPaused = !isPaused;
        }

        if (IsKeyPressed(KEY_F1)) {
            debugMode = !debugMode;
            TraceLog(LOG_INFO, "Debug mode %s", debugMode ? "enabled" : "disabled");
        }
            if (!isPaused) {

        Rectangle platforms[7 + SMALL_PLATFORM_COUNT];
        platforms[0] = floor;
        platforms[1] = bossFloor;
        platforms[2] = bossRoomL;
        platforms[3] = bossRoomR;
        platforms[6] = celling;
        if(CheckCollisionRecs((Rectangle){ player.base.hitbox.x - 150, player.base.hitbox.y,
                     player.base.hitbox.width, player.base.hitbox.height },
        Lgate) && Brute.base.isAlive) {
        gatesOpen = false;
        }
        if(!gatesOpen){
            platforms[4] = Lgate;
            platforms[5] = Rgate;  
        }
        for (int i = 0; i < SMALL_PLATFORM_COUNT; i++) {
        platforms[i + 7] = smallPlatforms[i];
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

        // Apply movement
        player.base.hitbox.x += player.base.velocity.x;


        // Physics
        updatePlayer(&player, &enemy, platforms);
        updateEnemy(&enemy, &player, platforms, 1, speed - 2, 5.0f);

        // Handle player–enemy collision
        

        // Reduce timers
        float dt = GetFrameTime();

        // If knockback active, reduce timer
        if (player.knkbackTime > 0) {
            player.knkbackTime -= GetFrameTime();
        }

        // Update bullet positions
        UpdateBullets(bullets, MAX_BULLETS);

        // Update camera position once
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

       // Background elements based on current zone
ClearBackground(zone->bgColor);
DrawRectangleRec(zone->floor, BLACK);
for (int i = 0; i < zone->platformCount; i++) {
    DrawRectangleRec(zone->smallPlatforms[i], PURPLE);
}


        // Game elements
        DrawBullets(bullets, MAX_BULLETS);  // Draw bullets before player
        
        // Player
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

        // Enemy and projectiles
        if (!enemy.base.isAlive) {
            // Death animation
            float alpha = 1.0f - (enemy.deathTimer / 1.0f);  // 1 second fade
            DrawRectangleRec(enemy.base.hitbox, Fade(RED, alpha));
        } else if (enemy.isCharging) {
            // Normal charging animation
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
            DrawRectangleLines(floor->x, floor->y, floor->width, floor->height, GREEN);
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

        EndMode2D();

// --- UI elements ---
for (int i = 0; i < player.maxHealth; i++) {
    Color heartColor = (i < player.health) ? RED : DARKGRAY;
    DrawRectangle(20 + i * 40, 20, 30, 30, heartColor);
}

// --- Boss name popup ---
if (Brute.showName) {
    Brute.nameTimer -= GetFrameTime();
    if (Brute.nameTimer <= 0) {
        Brute.showName = false;
    } else {
        float alpha = (Brute.nameTimer > 1.0f) ? 1.0f : Brute.nameTimer / 1.0f;
        Color nameColor = Fade(MAROON, alpha);

        const char *bossName = "THE BRUTE";
        int fontSize = 90;
        int padding = 20;

        Vector2 size = MeasureTextEx(titleFont, bossName, fontSize, 2);

        DrawTextEx(
            titleFont,   // ✅ use titleFont for both measuring and drawing
            bossName,
            (Vector2){ GetScreenWidth() - size.x - padding, GetScreenHeight() - fontSize - padding },
            fontSize,
            2,
            nameColor
        );
    }
}

        

        EndDrawing();
    }

    // Add cleanup at end of function
    free(smallPlatforms);
}
