#include "raylib.h"
#include "fonts.h"
#include "menu.h"
#include "sandbox.h"

void sandBox() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    Rectangle player = { screenWidth / 2.0f - 25, screenHeight / 2.0f - 25, 50, 50 };
    float speed = 5;

    // Setup camera for potential scrolling later
    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.x + player.width/2, player.y + player.height/2 };
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.zoom = 1.0f;

    while (currentScreen == SCREEN_SANDBOX && !WindowShouldClose()) {
        // ESC to quit to main menu
        if (IsKeyPressed(KEY_ESCAPE)) {
            TraceLog(LOG_INFO, "ESC pressed - returning to menu");
            currentScreen = SCREEN_MAIN_MENU;
            return;
        }

        // Movement
        if (IsKeyDown(KEY_RIGHT)) player.x += speed;
        if (IsKeyDown(KEY_LEFT))  player.x -= speed;
        if (IsKeyDown(KEY_DOWN))  player.y += speed;
        if (IsKeyDown(KEY_UP))    player.y -= speed;

        // Camera follows player
        camera.target = (Vector2){ player.x + player.width/2, player.y + player.height/2 };

        // Drawing
        BeginDrawing();
        ClearBackground(RED);

        BeginMode2D(camera);

        // Draw floor
        DrawRectangle(0, screenHeight - 100, screenWidth * 10, 100, GRAY); // Wide floor

        // Draw player
        DrawRectangleRec(player, BLUE);

        EndMode2D();

        DrawText("Press ESC to return to menu", 20, 20, 20, WHITE);
        EndDrawing();
    }
}
