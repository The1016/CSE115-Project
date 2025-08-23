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
#include "music.h"

#define DARKRED  (Color){139, 0, 0, 255} 

#define focusOffsetY 200  // Distance above player to center camera

bool debugMode = false;
int SMALL_PLATFORM_COUNT = 4;
float CONST_GRAVITY = 0.5f;
bool gatesOpen = true; 


Rectangle *smallPlatforms;
Bullet bullets[MAX_BULLETS];

extern GameScreen currentScreen;  // Declare external variable

void sandBox() {
    InitSandboxMusic();
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
    Vector2 respawnPoint = {
        player.base.hitbox.x,
        player.base.hitbox.y
    };

    Enemy enemy = {0}; // zero everything
    enemy.base.hitbox = (Rectangle){ player.base.hitbox.x - 200, player.base.hitbox.y, 35, 60 };
    enemy.base.velocity = (Vector2){ 0, 0 };
    enemy.base.onGround = false;
    enemy.base.health = 5;
    enemy.deathTimer = 0.0f;
    enemy.attackCooldown = 0.0f;  
    enemy.attackWindup = 0.0f;    
    enemy.isCharging = false;     
    enemy.base.damageCooldown = 0.0f;
    enemy.base.isAlive = true;

    chaser enemy1 = {0};
    enemy1.spawnPos.x = screenWidth / 2;
    enemy1.spawnPos.y = bossFloorTop ;
    enemy1.base.hitbox = (Rectangle){enemy1.spawnPos.x, enemy1.spawnPos.y, 35, 60};
    enemy1.base.velocity = (Vector2){0, 0};
    enemy1.isChasing = false;
    enemy1.patrolDirection = 1; 
    enemy1.patrolDistance = 200;

    boss1 Brute = {0};
    Brute.spawnPos.x = screenWidth * 3;
    Brute.spawnPos.y = bossFloorTop-60;   
    Brute.base.hitbox = (Rectangle){Brute.spawnPos.x, Brute.spawnPos.y, 35, 60};
    Brute.base.velocity = (Vector2){0, 0};
    Brute.isCharging = false;
    Brute.base.isAlive = true;
    Brute.isAwake = false;  // Boss starts asleep
    Brute.base.health = 10;
    Brute.showName = false;


    // Use the global camera
    Camera2D *camera = getGameCamera();
    camera->offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera->zoom = 1.0f;

    // Add bullet initialization after player init
    InitBullets(bullets, MAX_BULLETS);

    while (currentScreen == SCREEN_SANDBOX && !WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) {
        isPaused = !isPaused;
        }
        if(!isPaused){
            UpdateSandboxMusic();
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
        ShootBullet(bullets, dir, player.facingDirection);
        }

        // Apply movement
        player.base.hitbox.x += player.base.velocity.x;


        // Physics
        updatePlayer(&player, &enemy, platforms);
        updateEnemy(&enemy, &player, platforms, SMALL_PLATFORM_COUNT + 1, speed - 2, 5.0f);
        handleSlash(&player, &Brute.base);
        handleHealing(&player);
        handlePlayerCollisionDamage(&player, &enemy.base, 1);


        // Handle player–enemy collision
        

        // Reduce timers
        float dt = GetFrameTime();

        // If knockback active, reduce timer
        if (player.knkbackTime > 0) {
            player.knkbackTime -= GetFrameTime();
        }
        if (player.base.hitbox.y > floorBottom + 200) {
                player.base.health -= 1; // Take 1 damage
                if (player.base.health > 0) {
                    // Respawn at checkpoint
                    player.base.hitbox.x = respawnPoint.x;
                    player.base.hitbox.y = respawnPoint.y;
                    player.base.velocity = (Vector2){0, 0};
                } else {
                    player.isAlive = false;
                }
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
        Vector2 cameraTarget;
        if (!gatesOpen && Brute.base.isAlive) {
            // Boss fight: lock horizontally in boss room, but keep Y as desiredY
            float bossRoomCenterX = (bossRoomL.x + bossRoomR.x + bossRoomR.width) / 2.0f;
            cameraTarget = (Vector2){ bossRoomCenterX, desiredY };
        } else {
            // Normal camera following player
            cameraTarget = (Vector2){ player.base.hitbox.x + player.base.hitbox.width / 2, desiredY };
        }

        camera->target = cameraTarget;

        if(!Brute.base.isAlive){
            gatesOpen = true;
        }

        // Drawing - reorganized for proper order
        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(*camera);  // Use pointer dereference
        //Chasers(&enemy1, &player, platforms, SMALL_PLATFORM_COUNT + 1);
        // Background elements
        DrawRectangleRec(floor, BLACK);
        DrawRectangleRec(bossFloor, RAYWHITE);
        DrawRectangleRec(bossRoomL, RAYWHITE);
        DrawRectangleRec(bossRoomR, RAYWHITE);
        DrawRectangleRec(celling, RAYWHITE);
        if(!gatesOpen){
            DrawRectangleRec(Lgate, RED);
            DrawRectangleRec(Rgate, RED);
        }
        Boss1(&Brute, &player, platforms, SMALL_PLATFORM_COUNT + 6);
        for (int i = 0; i < SMALL_PLATFORM_COUNT; i++) {
            DrawRectangleRec(smallPlatforms[i], PURPLE);
        }

        // Game elements
        DrawBullets(bullets, MAX_BULLETS);  // Draw bullets before player

// --- Player Drawing ---
if (player.isAlive) {
    Color playerColor = RAYWHITE;

    // If damage cooldown (IFrames) is active → flicker
    if (player.base.damageCooldown > 0.0f) {
        float t = GetTime() * 18.0f;  // speed of flicker
        float alpha = (sinf(t) > 0) ? 0.3f : 1.0f;  
        playerColor.a = (unsigned char)(alpha * 255); 
    }

    DrawRectangleRec(player.base.hitbox, playerColor);

    // Healing effect (only if alive)
    if (player.isHealing) {
        float centerX = player.base.hitbox.x + player.base.hitbox.width / 2.0f;
        float centerY = player.base.hitbox.y + player.base.hitbox.height / 2.0f;

        float pulse = 5.0f * sinf(GetTime() * 10); 
        DrawCircle(centerX, centerY, 30 + pulse, (Color){255, 255, 200, 100});
        DrawCircle(centerX, centerY, 20, (Color){255, 255, 150, 150});
    }
}

else {
    // Player is dead → draw death animation
    float deathDuration = 2.0f;  // same as in updatePlayer()
    float alpha = 1.0f - (player.deathTimer / deathDuration);
    if (alpha < 0) alpha = 0;

    // Fading out body
    DrawRectangleRec(player.base.hitbox, Fade(RED, alpha));

    // Optional: add a glow or shrinking effect for flair
    float shrink = player.base.hitbox.width * (1.0f - (player.deathTimer / deathDuration) * 0.5f);
    DrawCircle(player.base.hitbox.x + player.base.hitbox.width / 2.0f,
               player.base.hitbox.y + player.base.hitbox.height / 2.0f,
               shrink, Fade(DARKRED, alpha));
}




        // Slash effect
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
            DrawRectangleLines(floor.x, floor.y, floor.width, floor.height, GREEN);
            DrawRectangleLines(bossFloor.x, bossFloor.y, bossFloor.width, bossFloor.height, GREEN);
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
        EndMode2D();

// --- Flask (big circle) ---
int flaskX = 60;
int flaskY = 60;
int flaskRadius = 45;

float fillRatio = player.mana / player.maxMana;
Color flaskColor = (fillRatio > 0.0f) ? SKYBLUE : DARKGRAY;

// Draw filled part (mana progress)
DrawCircleSector((Vector2){flaskX, flaskY}, flaskRadius, 0, fillRatio * 360, 0, flaskColor);

// Outline
DrawCircleLines(flaskX, flaskY, flaskRadius, BLACK);

// --- Hearts ---
for (int i = 0; i < player.maxHealth; i++) {
    Color heartColor = (i < player.base.health) ? RED : DARKGRAY;
    DrawRectangle(flaskX + flaskRadius + 20 + i * 40, flaskY - 20, 30, 30, heartColor);
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
    StopSandboxMusic();
    // Add cleanup at end of function
    free(smallPlatforms);
}