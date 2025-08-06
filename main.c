// main.c
#include "raylib.h"
#include <stdio.h>
#include <stdarg.h>
#include "fonts.h"
#include "menu.h" // includes GameScreen and extern currentScreen

// Shared screen state
GameScreen currentScreen = SCREEN_MAIN_MENU;

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
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(screenWidth, screenHeight, "Menu System");
    
    SetTargetFPS(60);
    SetTraceLogCallback(CustomLog);

    LoadAllFonts();

    while (!WindowShouldClose() && currentScreen != SCREEN_EXIT) {
        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentScreen) {
            case SCREEN_MAIN_MENU:
                ParentMenu();
                break;
            case SCREEN_DIFFICULTY_MENU:
                diffMenu();
                break;
            default:
                break;
        }

        EndDrawing();
    }

    UnloadAllFonts();
    CloseWindow();
    return 0;
}
