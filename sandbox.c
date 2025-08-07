#include "raylib.h"
#include "fonts.h"
#include "menu.h"
#include "sandbox.h"
#include "phy.h"


#define grav 0.5f // Gravity constant
#define jump_velocity -10.0f // Jump velocity
#define max_jumptime 0.25f // Maximum time for jump hold

// Global debug mode flag
bool debugMode = false;
bool debugMode; // Global debug mode flag


void sandBox() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    
    float speed = 5;
    Rectangle floor = { 0, screenHeight - 100, screenWidth * 10, 100 }; // Wide floor

    // Setup camera for potential scrolling later
    

    //Entity setup
    Entity player = {
    .hitbox = { screenWidth / 2.0f - 25, screenHeight / 2.0f - 25, 35, 60 },
    .velocity = { 0, 0 },
    .onGround = false
    };

    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.hitbox.x + player.hitbox.width/2, player.hitbox.y + player.hitbox.height/2 };
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.zoom = 1.0f;



    while (currentScreen == SCREEN_SANDBOX && !WindowShouldClose()) {
        // ESC to quit to main menu
        if (IsKeyPressed(KEY_ESCAPE)) {
            TraceLog(LOG_INFO, "ESC pressed - returning to menu");
            currentScreen = SCREEN_MAIN_MENU;
            return;
        }
        if (IsKeyPressed(KEY_F1)){
            debugMode = !debugMode; // Toggle debug mode
            TraceLog(LOG_INFO, "Debug mode %s", debugMode ? "enabled" : "disabled");
        }
        
        
        Rectangle platform[] = {floor};
        //physics
        handleDash(&player); 
        handleJump(&player); 
        applyGravity(&player, grav);
        updateEntity(&player, platform, 1);


        // Movement
        // Regular movement only if not dashing
        if (!player.isDashing) {
            player.velocity.x = 0; // Reset before applying input
            if (IsKeyDown(KEY_D)) player.velocity.x += speed;
            if (IsKeyDown(KEY_A)) player.velocity.x -= speed;
        }

        // Apply movement
        player.hitbox.x += player.velocity.x;
        // Camera follows player
        camera.target = (Vector2){ player.hitbox.x + player.hitbox.width/2, player.hitbox.y + player.hitbox.height/2 };

        // Drawing
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        // Draw floor
        DrawRectangleRec(floor, BLACK);

        // Draw player
        DrawRectangleRec(player.hitbox, RAYWHITE);

        if(debugMode){    
            DrawRectangleLines(player.hitbox.x, player.hitbox.y, player.hitbox.width, player.hitbox.height, BLUE);
            DrawRectangleLines(floor.x, floor.y, floor.width, floor.height, GREEN);
        }

        EndMode2D();

        DrawText("Press ESC to return to menu", 20, 20, 20, WHITE);
        EndDrawing();
    }
}
