#include "raylib.h"
#include "fonts.h"
#include "menu.h"
#include "sandbox.h"
#include "phy.h"
#include "gun.h"
#include "zones.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define focusOffsetY 200

// --- extern globals from sandbox.c ---
extern Rectangle *smallPlatforms;
extern Bullet bullets[MAX_BULLETS];

// Story intro state
static bool storyIntroPlayed = false;
static int currentLine = 0;
static const char *storyText[] = {
    "Welcome to Story Mode!",
    "Your adventure begins...",
    "Explore the zones and defeat enemies!",
    "Press SPACE to start playing, ESC to exit."
};
static int storyLines = sizeof(storyText) / sizeof(storyText[0]);

void storyMode() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Initialize zones
    initZones(screenWidth, screenHeight);
    currentZone = 0;

    float speed = 5;

    MapZone* zone = &zones[currentZone];
    Rectangle* floor = &zone->floor;
    smallPlatforms = zone->smallPlatforms;
    int SMALL_PLATFORM_COUNT = zone->platformCount;
    Color bgColor = zone->bgColor;

    Player player = {0};
    initializePlayer(&player, screenWidth, screenHeight);

    Enemy enemy = {0};
    enemy.base.hitbox = (Rectangle){ player.base.hitbox.x - 200, player.base.hitbox.y, 35, 60 };
    enemy.base.velocity = (Vector2){ 0, 0 };
    enemy.base.onGround = false;

    Camera2D *camera = getGameCamera();
    camera->offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera->zoom = 1.0f;

    InitBullets(bullets, MAX_BULLETS);

    while (currentScreen == SCREEN_STORY && !WindowShouldClose()) {

        // --- Story intro input ---
        if (!storyIntroPlayed) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_MAIN_MENU;
                storyIntroPlayed = false;
                currentLine = 0;
                return;
            }
            if (IsKeyPressed(KEY_SPACE)) {
                currentLine++;
                if (currentLine >= storyLines) {
                    storyIntroPlayed = true;
                    currentLine = 0;
                }
            }

            BeginDrawing();
            ClearBackground(BLACK);
            int fontSize = 40;
            if (currentLine < storyLines) {
                int textWidth = MeasureText(storyText[currentLine], fontSize);
                DrawText(storyText[currentLine], screenWidth/2 - textWidth/2, screenHeight/2 - fontSize, fontSize, WHITE);
            }
            EndDrawing();
            continue; // skip rest of loop until intro done
        }

        // --- Sandbox-style gameplay after story ---
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentScreen = SCREEN_MAIN_MENU;
            return;
        }

        Rectangle platforms[1 + SMALL_PLATFORM_COUNT];
        platforms[0] = *floor;
        for (int i = 0; i < SMALL_PLATFORM_COUNT; i++)
            platforms[i + 1] = smallPlatforms[i];

        // Player movement
        if (!player.isDashing && player.knkbackTime <= 0.0f) {
            player.base.velocity.x = 0;
            if (IsKeyDown(KEY_D)) player.base.velocity.x += speed;
            if (IsKeyDown(KEY_A)) player.base.velocity.x -= speed;
        }

        if (IsKeyPressed(KEY_M)) {
            Vector2 startPos = {
                player.base.hitbox.x + player.base.hitbox.width,
                player.base.hitbox.y + player.base.hitbox.height / 2
            };
            Vector2 dir = {10, 0};
            ShootBullet(bullets, startPos, dir);
        }

        // Apply movement and bounds
        player.base.hitbox.x += player.base.velocity.x;
        if (player.base.hitbox.x < zones[currentZone].leftBound)
            player.base.hitbox.x = zones[currentZone].leftBound;
        if (player.base.hitbox.x + player.base.hitbox.width > zones[currentZone].rightBound)
            player.base.hitbox.x = zones[currentZone].rightBound - player.base.hitbox.width;

        updatePlayer(&player, &enemy, platforms);
        updateEnemy(&enemy, &player, platforms, 1, speed - 2, 5.0f);
        UpdateBullets(bullets, MAX_BULLETS);

        // Camera follow
        float desiredY = player.base.hitbox.y + player.base.hitbox.height / 2 - focusOffsetY;
        float floorBottom = floor->y + floor->height;
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
                player.base.hitbox.x = 50;
                player.base.hitbox.y = screenHeight / 2;
                zone = &zones[currentZone];
                floor = &zone->floor;
                smallPlatforms = zone->smallPlatforms;
                SMALL_PLATFORM_COUNT = zone->platformCount;
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground(zone->bgColor);
        BeginMode2D(*camera);

        DrawRectangleRec(zone->floor, BLACK);
        for (int i = 0; i < zone->platformCount; i++)
            DrawRectangleRec(zone->smallPlatforms[i], PURPLE);

        DrawBullets(bullets, MAX_BULLETS);
        DrawRectangleRec(player.base.hitbox, player.isAlive ? RAYWHITE : Fade(RAYWHITE, 0.5f));
        DrawRectangleRec(enemy.base.hitbox, RED);

        EndMode2D();

        // UI
        for (int i = 0; i < player.maxHealth; i++) {
            DrawRectangle(20 + i * 40, 20, 30, 30, (i < player.health) ? RED : DARKGRAY);
        }

        EndDrawing();
    }
}
