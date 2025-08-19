#include "raylib.h"
#include "fonts.h"
#include "menu.h"
#include "sandbox.h"
#include "phy.h"
#include "gun.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "zones.h"


#define focusOffsetY 200  // Distance above player to center camera

bool debugMode = false;
int SMALL_PLATFORM_COUNT = 4;
float CONST_GRAVITY = 0.5f;


Rectangle *smallPlatforms;
Bullet bullets[MAX_BULLETS];

void sandBox() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    initZones(screenWidth, screenHeight);
    currentZone = 0;

    float speed = 5;
    
   // Use current zone for floor & platforms
MapZone* zone = &zones[currentZone];   // zones[] is from zones.c
Rectangle* floor = &zone->floor;
smallPlatforms = zone->smallPlatforms; // assign pointer
SMALL_PLATFORM_COUNT = zone->platformCount;
Color bgColor = zone->bgColor;


    Player player = {0};
    initializePlayer(&player, screenWidth, screenHeight);

    Enemy enemy = {0}; // zero everything
    enemy.base.hitbox = (Rectangle){ player.base.hitbox.x - 200, player.base.hitbox.y, 35, 60 };
    enemy.base.velocity = (Vector2){ 0, 0 };
    enemy.base.onGround = false;

    float floorBottom = floor->y + floor->height;
float cameraMinY = floorBottom - screenHeight / 2.0f;


    // Use the global camera
    Camera2D *camera = getGameCamera();
    camera->offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera->zoom = 1.0f;


    // Add bullet initialization after player init
    InitBullets(bullets, MAX_BULLETS);

    while (currentScreen == SCREEN_SANDBOX && !WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            TraceLog(LOG_INFO, "ESC pressed - returning to menu");
            currentScreen = SCREEN_MAIN_MENU;
            return;
        }
        if (IsKeyPressed(KEY_F1)) {
            debugMode = !debugMode;
            TraceLog(LOG_INFO, "Debug mode %s", debugMode ? "enabled" : "disabled");
        }

        Rectangle platforms[1 + SMALL_PLATFORM_COUNT];
        platforms[0] = *floor;
        for (int i = 0; i < SMALL_PLATFORM_COUNT; i++) {
        platforms[i + 1] = smallPlatforms[i];
        }

        // Movement
        // Regular movement only if not dashing
        if (!player.isDashing && player.knkbackTime <= 0.0f) {
            // only clear & apply input movement if not currently in knockback
            player.base.velocity.x = 0;
            if (IsKeyDown(KEY_D)) player.base.velocity.x += speed;
            if (IsKeyDown(KEY_A)) player.base.velocity.x -= speed;
        }
         if (IsKeyPressed(KEY_M)) {
        Vector2 startPos = {
        player.base.hitbox.x + player.base.hitbox.width,
        player.base.hitbox.y + player.base.hitbox.height / 2
        };
        Vector2 dir = { 10, 0 };  // Move right at speed 10
        ShootBullet(bullets, startPos, dir);
        }

        // Apply movement
        player.base.hitbox.x += player.base.velocity.x;
        if (player.base.hitbox.x < zones[currentZone].leftBound)
    player.base.hitbox.x = zones[currentZone].leftBound;
if (player.base.hitbox.x + player.base.hitbox.width > zones[currentZone].rightBound)
    player.base.hitbox.x = zones[currentZone].rightBound - player.base.hitbox.width;



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
        // Zone transition
if (CheckCollisionRecs(player.base.hitbox, zone->entrance)) {
    if (currentZone < MAX_ZONES - 1) {
        currentZone++;
        player.base.hitbox.x = 50;              // Starting X in new zone
        player.base.hitbox.y = GetScreenHeight() / 2; // Starting Y
        zone = &zones[currentZone];            // Update current zone pointer
    }
}


        // Drawing - reorganized for proper order
        BeginDrawing();
        ClearBackground(MAROON);
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

        // Slash effect
        if (player.isSlashing) {
            float alpha = 1.0f - (player.slashTimer / player.slashDuration);
            DrawRectangleRec(player.slashHitbox, Fade(YELLOW, alpha));
        }

        // Enemy and projectiles
        if (enemy.isCharging) {
            // Charging animation (pulsing effect)
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

        // Draw enemy projectiles
        for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
            if (enemy.projectiles[i].active) {
                DrawRectangleRec(enemy.projectiles[i].hitbox, ORANGE);
                
                // Draw projectile outline in debug mode
                if (debugMode) {
                    DrawRectangleLinesEx(enemy.projectiles[i].hitbox, 1, RED);
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
        }
        }
        if (!player.isAlive && player.deathTimer >= 2.0f) {
            currentScreen = SCREEN_MAIN_MENU; // After 2s fade
            TraceLog(LOG_INFO, "Player death animation done - returning to main menu");
        }

        EndMode2D();

        // UI elements (after EndMode2D)
        for (int i = 0; i < player.maxHealth; i++) {
            Color heartColor = (i < player.health) ? RED : DARKGRAY;
            DrawRectangle(20 + i * 40, 20, 30, 30, heartColor);
        }
// Draw current zone location
DrawRectangle(15, 55, 150, 30, Fade(BLACK, 0.5f));
DrawText(TextFormat("Location: %s", zone->name), 20, 60, 20, WHITE);

        EndDrawing();
    }

    // Add cleanup at end of function
   
}
