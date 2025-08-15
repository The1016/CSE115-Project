// main.c
#include "raylib.h"
#include <stdio.h>
#include <stdarg.h>
#include "fonts.h"
#include "menu.h" // includes GameScreen and extern currentScreen
#include "sandbox.h" // for sandBox function


// Shared screen state
GameScreen currentScreen = SCREEN_MAIN_MENU;
bool isPaused = false;


void CustomLog(int logLevel, const char *text, va_list args) {
    FILE *logFile = fopen("log.txt", "a");
    if (logFile) {
        const char *levelStr = "";
        switch (logLevel) {
            case LOG_INFO: levelStr = "INFO"; break;
            case LOG_WARNING: levelStr = "WARNING"; break;
            case LOG_ERROR: levelStr = "ERROR"; break;
            case LOG_DEBUG: levelStr = "DEBUG"; break;
            case LOG_FATAL: levelStr = "FATAL"; break;
            default: levelStr = "UNKNOWN"; break;
        }

        fprintf(logFile, "[%s] ", levelStr);
        vfprintf(logFile, text, args);
        fprintf(logFile, "\n");
        fclose(logFile);
    }

    vprintf(text, args);
    printf("\n");
}

int main() {
    int screenWidth = GetMonitorWidth(0);
    int screenHeight = GetMonitorHeight(0);
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_MAXIMIZED);
    InitWindow(screenWidth, screenHeight, "Menu System");

    SetExitKey(0);
    
    SetTargetFPS(60);
    SetTraceLogCallback(CustomLog);

    LoadAllFonts();

    while (!WindowShouldClose() && currentScreen != SCREEN_EXIT) {
    switch (currentScreen) {
        case SCREEN_MAIN_MENU:
            BeginDrawing();
            ClearBackground(BLACK);
            ParentMenu();
            EndDrawing();
            break;
        case SCREEN_DIFFICULTY_MENU:
            BeginDrawing();
            ClearBackground(BLACK);
            diffMenu();
            EndDrawing();
            break;
        case SCREEN_GAMEMODE:
            BeginDrawing();
            ClearBackground(BLACK);
            gamemodeMenu();
            EndDrawing();
            break;
        case SCREEN_GAME:
            if (selectedGamemode == 0) {
                TraceLog(LOG_INFO, "Starting Story mode...");
                currentScreen = SCREEN_MAIN_MENU;
            } else if (selectedGamemode == 1) {
                TraceLog(LOG_INFO, "Starting Cybergrid mode...");
                currentScreen = SCREEN_MAIN_MENU;
            } else if (selectedGamemode == 2) {
                TraceLog(LOG_INFO, "Starting Sandbox mode...");
                currentScreen = SCREEN_SANDBOX;
                sandBox();
                continue; // This handles its own drawing
            }
            break;
        default:
            break;
    }
}
 

    UnloadAllFonts();
    CloseWindow();
    return 0;
}
