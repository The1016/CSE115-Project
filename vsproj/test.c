#include "raylib.h"
#include <stdio.h>
#include <stdarg.h>

#define BUTTON_COUNT 5

void CustomLog(int logLevel, const char *text, va_list args)
{
    FILE *logFile = fopen("log.txt", "a");
    if (logFile)
    {
        const char *levelStr = "";
        switch (logLevel)
        {
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

const char *buttonLabels[BUTTON_COUNT] = {
    "NEW GAME",
    "CONTINUE",
    "SETTINGS",
    "CREDITS",
    "EXIT",
};

int main(void)
{
    SetTraceLogCallback(CustomLog);
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(0, 0, "Raylib Main Menu");

    // Load custom font
    Font customFont = LoadFontEx("assets/CinzelDecorative-Regular.ttf", 150, 0, 0);
    GenTextureMipmaps(&customFont.texture);
    SetTextureFilter(customFont.texture, TEXTURE_FILTER_TRILINEAR);

    Font tittleFont = LoadFontEx("assets/CinzelDecorative-Regular.ttf", 140, 0, 0);
    GenTextureMipmaps(&tittleFont.texture);
    SetTextureFilter(tittleFont.texture, TEXTURE_FILTER_TRILINEAR);

    Font msgFont = LoadFontEx("assets/CinzelDecorative-Regular.ttf", 30, 0, 0);
    GenTextureMipmaps(&msgFont.texture);
    SetTextureFilter(msgFont.texture, TEXTURE_FILTER_TRILINEAR);

    Font menuFont = LoadFontEx("assets/CinzelDecorative-Regular.ttf", 50, 0, 0);
    GenTextureMipmaps(&menuFont.texture);
    SetTextureFilter(menuFont.texture, TEXTURE_FILTER_TRILINEAR);
        if (customFont.texture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load custom font!");
        CloseWindow();
        return 1;
    }

    SetTargetFPS(60);

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    int buttonWidth = 300;
    int buttonHeight = 80;
    int buttonSpacing = 30;
    float fontSize = 40;
    float titleFontSize = 120;

    int totalButtonsHeight = BUTTON_COUNT * buttonHeight + (BUTTON_COUNT - 1) * buttonSpacing;
    int startY = (screenHeight - totalButtonsHeight) / 2 + 100;

    int selectedButton = -1;

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();

        BeginDrawing();
        ClearBackground(BLACK);

        // --- Draw Title ---
        const char *title = "MAIN MENU";
        Vector2 titleSize = MeasureTextEx(tittleFont, title, titleFontSize, 2);
        Vector2 titlePos = {
            (screenWidth - titleSize.x) / 2,
            60
        };
        DrawTextEx(tittleFont, title, titlePos, titleFontSize, 2, WHITE);

        // --- Draw Buttons ---
        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            Rectangle btn = {
                screenWidth / 2.0f - buttonWidth / 2.0f,
                startY + i * (buttonHeight + buttonSpacing),
                buttonWidth,
                buttonHeight
            };

            bool hovered = CheckCollisionPointRec(mouse, btn);
            Color btnColor = hovered ? LIGHTGRAY : DARKGRAY;
            DrawRectangleRec(btn, btnColor);

            // Draw label centered
            const char *label = buttonLabels[i];
            Vector2 labelSize = MeasureTextEx(menuFont, label, fontSize, 2);
            Vector2 labelPos = {
                btn.x + (btn.width - labelSize.x) / 2,
                btn.y + (btn.height - labelSize.y) / 2
            };
            DrawTextEx(menuFont, label, labelPos, fontSize, 2, WHITE);

            if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                selectedButton = i;
        }

        // --- Handle Button Actions ---
        if (selectedButton != -1)
        {
            switch (selectedButton)
            {
                case 0: TraceLog(LOG_INFO, "NEW GAME PRESSED"); break;
                case 1: TraceLog(LOG_INFO, "CONTINUE PRESSED"); break;
                case 2: TraceLog(LOG_INFO, "SETTINGS PRESSED"); break;
                case 3: TraceLog(LOG_INFO, "CREDITS PRESSED"); break;
                case 4:
                    TraceLog(LOG_INFO, "EXIT PRESSED");
                    CloseWindow();
                    return 0;
            }
            selectedButton = -1;
        }

        EndDrawing();
    }

    UnloadFont(customFont);
    UnloadFont(tittleFont);
    UnloadFont(msgFont);
    UnloadFont(menuFont);
    CloseWindow();
    return 0;
}
