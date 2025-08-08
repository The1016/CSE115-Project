#include "raylib.h"
#include "fonts.h"
#include "menu.h"
#include "sandbox.h"
#include "phy.h"
#include "gun.h"

bool debugMode = false;
#define SMALL_PLATFORM_COUNT 4

void updateEnemy(Entity *enemy, Entity *player, Rectangle *platforms, int platformCount, float chaseSpeed, float chaseThreshold);

void sandBox() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float speed = 5;
    
    Rectangle floor = { 0, screenHeight - 100, screenWidth * 10, 150 };
    float floorTop = floor.y;
    float floorBottom = floor.y + floor.height;
    
    Rectangle smallPlatforms[SMALL_PLATFORM_COUNT] = {
        { screenWidth / 3, floorTop - 190, 100, 20 },
        { screenWidth / 2, floorTop - 290, 100, 20 },
        { screenWidth / 4, floorTop - 340, 100, 20 },
        { screenWidth / 1.5f, floorTop - 240, 100, 20 }
    };
        

    Entity player = {
        .hitbox = { screenWidth / 2.0f - 25, screenHeight / 2.0f - 25, 35, 60 },
        .velocity = { 0, 0 },
        .onGround = false
    };

    Entity enemy = {
        .hitbox = { player.hitbox.x - 200, player.hitbox.y, 35, 60 },
        .velocity = { 0, 0 },
        .onGround = false
    };
     Bullet bullets[MAX_BULLETS];
    InitBullets(bullets, MAX_BULLETS);

    Camera2D camera = { 0 };
    float focusOffsetY = 50; // shift camera to focus higher above player
    float minCameraY = screenHeight / 2.0f - 100; // don’t go below floor view

    float desiredY = player.hitbox.y + player.hitbox.height / 2 - focusOffsetY;
    if (desiredY < minCameraY) desiredY = minCameraY;

    camera.target = (Vector2){
        player.hitbox.x + player.hitbox.width / 2,
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
        if (!player.isDashing) {
            player.velocity.x = 0; // Reset before applying input
            if (IsKeyDown(KEY_D)) player.velocity.x += speed;
            if (IsKeyDown(KEY_A)) player.velocity.x -= speed;
        }
         if (IsKeyPressed(KEY_M)) {
        Vector2 startPos = {
        player.hitbox.x + player.hitbox.width,
        player.hitbox.y + player.hitbox.height / 2
        };
        Vector2 dir = { 10, 0 };  // Move to the right
        ShootBullet(bullets, startPos, dir);
        }

        // Apply movement
        player.hitbox.x += player.velocity.x;


        //Physics
        handleDash(&player);
        handleJump(&player);
        applyGravity(&player, 0.5f);
        updateEntity(&player, platforms, 1 + SMALL_PLATFORM_COUNT);
        UpdateBullets(bullets, MAX_BULLETS);


        // Enemy movement
        updateEnemy(&enemy, &player, platforms, 1, speed - 2, 5.0f);

        // Update camera to follow player
        // Recalculate desired camera Y
        float desiredY = player.hitbox.y + player.hitbox.height / 2 - focusOffsetY;

        // Calculate lowest camera Y allowed (so camera doesn't show below floor)
        float cameraMinY = floorBottom - screenHeight / 2.0f;

        // Clamp
        if (desiredY > cameraMinY) desiredY = cameraMinY;

        camera.target = (Vector2){
            player.hitbox.x + player.hitbox.width / 2,
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
        DrawRectangleRec(player.hitbox, RAYWHITE);
        DrawRectangleRec(enemy.hitbox, RED);
        DrawBullets(bullets, MAX_BULLETS);


        if (debugMode) {
            DrawRectangleLines(player.hitbox.x, player.hitbox.y, player.hitbox.width, player.hitbox.height, BLUE);
            DrawRectangleLines(floor.x, floor.y, floor.width, floor.height, GREEN);
            DrawRectangleLines(enemy.hitbox.x, enemy.hitbox.y, enemy.hitbox.width, enemy.hitbox.height, ORANGE);
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
