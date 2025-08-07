#include "raylib.h"
#include "fonts.h"
#include "menu.h"
#include "sandbox.h"
#include "phy.h"

bool debugMode = false;

void updateEnemy(Entity *enemy, Entity *player, Rectangle *platforms, int platformCount, float chaseSpeed, float chaseThreshold);

void sandBox() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float speed = 5;
    Rectangle floor = { 0, screenHeight - 100, screenWidth * 10, 100 };

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

    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.hitbox.x + player.hitbox.width / 2, player.hitbox.y + player.hitbox.height / 2 };
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

        Rectangle platforms[] = { floor };

        // Player movement input
        player.velocity.x = 0;
        if (IsKeyDown(KEY_D)) player.velocity.x = speed;
        if (IsKeyDown(KEY_A)) player.velocity.x = -speed;

        if (IsKeyDown(KEY_SPACE) && player.onGround) {
            player.velocity.y = -10; // jump
        }

        applyGravity(&player, 0.5f);
        updateEntity(&player, platforms, 1);

        // Enemy movement
        updateEnemy(&enemy, &player, platforms, 1, speed - 2, 5.0f);

        // Update camera to follow player
        camera.target = (Vector2){ player.hitbox.x + player.hitbox.width / 2, player.hitbox.y + player.hitbox.height / 2 };

        // Drawing
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        DrawRectangleRec(floor, BLACK);
        DrawRectangleRec(player.hitbox, RAYWHITE);
        DrawRectangleRec(enemy.hitbox, RED);

        if (debugMode) {
            DrawRectangleLines(player.hitbox.x, player.hitbox.y, player.hitbox.width, player.hitbox.height, BLUE);
            DrawRectangleLines(floor.x, floor.y, floor.width, floor.height, GREEN);
            DrawRectangleLines(enemy.hitbox.x, enemy.hitbox.y, enemy.hitbox.width, enemy.hitbox.height, ORANGE);
        }

        EndMode2D();

        DrawText("Press ESC to return to menu", 20, 20, 20, WHITE);
        EndDrawing();
    }
}
