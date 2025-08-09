#include "raylib.h"
#include "fonts.h"
#include "menu.h"
#include "sandbox.h"
#include "phy.h"
#include "gun.h"
#include <stdlib.h>
#include <stdio.h>

bool debugMode = false;
int SMALL_PLATFORM_COUNT = 4;
float CONST_GRAVITY = 0.5f;


Rectangle *smallPlatforms;
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
        

    Player player = {0}; // zero everything

    player.base.hitbox = (Rectangle){ screenWidth / 2.0f - 25, screenHeight / 2.0f - 25, 35, 60 };
    player.base.velocity = (Vector2){ 0, 0 };
    player.base.onGround = false;

    Enemy enemy = {0}; // zero everything
        enemy.base.hitbox = (Rectangle){ player.base.hitbox.x - 200, player.base.hitbox.y, 35, 60 },
        enemy.base.velocity = (Vector2){ 0, 0 },
        enemy.base.onGround = false;
 
    Bullet bullets[MAX_BULLETS];
    InitBullets(bullets, MAX_BULLETS);

    Camera2D camera = { 0 };
    float focusOffsetY = 50; // shift camera to focus higher above player
    float minCameraY = screenHeight / 2.0f - 100; // don’t go below floor view

    float desiredY = player.base.hitbox.y + player.base.hitbox.height / 2 - focusOffsetY;
    if (desiredY < minCameraY) desiredY = minCameraY;

    camera.target = (Vector2){
        player.base.hitbox.x + player.base.hitbox.width / 2,
        desiredY
    };
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.zoom = 1.0f;

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
        platforms[0] = floor;
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
        Vector2 dir = { 10, 0 };  // Move to the right
        ShootBullet(bullets, startPos, dir);
        }

        // Apply movement
        player.base.hitbox.x += player.base.velocity.x;


        // Physics
        handleDash(&player);
        handleJump(&player);
        updateEntity(&player.base, platforms, 1 + SMALL_PLATFORM_COUNT, player.knkbackTime);
        updateEnemy(&enemy, &player, platforms, 1, speed - 2, 5.0f);

        // Handle player–enemy collision
        resolveEntityCollision(&player, &enemy);

        // Reduce timers
        float dt = GetFrameTime();
        if (player.iFrames > 0) player.iFrames -= dt;
        if (player.knkbackTime > 0) player.knkbackTime -= dt;

        // If knockback active, reduce timer
        if (player.knkbackTime > 0) {
            player.knkbackTime -= GetFrameTime();
        }

        

        // Update camera to follow player
        // Recalculate desired camera Y
        float desiredY = player.base.hitbox.y + player.base.hitbox.height / 2 - focusOffsetY;

        // Calculate lowest camera Y allowed (so camera doesn't show below floor)
        float cameraMinY = floorBottom - screenHeight / 2.0f;

        // Clamp
        if (desiredY > cameraMinY) desiredY = cameraMinY;

        camera.target = (Vector2){
            player.base.hitbox.x + player.base.hitbox.width / 2,
            desiredY
        };

        // Drawing
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        DrawRectangleRec(floor, BLACK);
        for (int i = 0; i < SMALL_PLATFORM_COUNT; i++) {
        DrawRectangleRec(smallPlatforms[i], PURPLE);
        }
        DrawRectangleRec(player.base.hitbox, RAYWHITE);
        DrawRectangleRec(enemy.base.hitbox, RED);
        DrawBullets(bullets, MAX_BULLETS);


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
        }
        }

        EndMode2D();

        DrawText("Press ESC to return to menu", 20, 20, 20, WHITE);
        EndDrawing();
    }
}
