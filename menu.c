// menu.c
#include "raylib.h"
#include "fonts.h"
#include "menu.h"

#define PARENT_BUTTON_COUNT 5
#define DIFF_BUTTON_COUNT 3
#define GAMEMODE_COUNT 3

float fontSize = 40;
float titleFontSize = 120;
int buttonWidth = 300;
int buttonHeight = 80;
int buttonSpacing = 30;
int selectedDifficulty = -1; // in diffMenu()
int selectedGamemode = -1;   // in gamemodeMenu()
const char *buttonLabels[PARENT_BUTTON_COUNT] = {
    "NEW GAME", "CONTINUE", "SETTINGS", "CREDITS", "EXIT"
};

const char *diffLabels[DIFF_BUTTON_COUNT] = {
    "SENILE", "HUMAN", "DEITY"
};

const char *gamemodeLabel[GAMEMODE_COUNT] = {
    "STORY MODE", "CYBERGRID", "SANDBOX"
};

void ParentMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int totalButtonsHeight = PARENT_BUTTON_COUNT * buttonHeight + (PARENT_BUTTON_COUNT - 1) * buttonSpacing;
    int startY = (screenHeight - totalButtonsHeight) / 2 + 100;

    Vector2 mouse = GetMousePosition();

    const char *title = "MAIN MENU";
    Vector2 titleSize = MeasureTextEx(titleFont, title, titleFontSize, 2);
    Vector2 titlePos = { (screenWidth - titleSize.x) / 2, 60 };
    DrawTextEx(titleFont, title, titlePos, titleFontSize, 2, WHITE);

    for (int i = 0; i < PARENT_BUTTON_COUNT; i++) {
        Rectangle btn = {
            screenWidth / 2.0f - buttonWidth / 2.0f,
            startY + i * (buttonHeight + buttonSpacing),
            buttonWidth, buttonHeight
        };

        bool hovered = CheckCollisionPointRec(mouse, btn);
        Color btnColor = hovered ? LIGHTGRAY : DARKGRAY;
        DrawRectangleRec(btn, btnColor);

        Vector2 labelSize = MeasureTextEx(menuFont, buttonLabels[i], fontSize, 2);
        Vector2 labelPos = {
            btn.x + (btn.width - labelSize.x) / 2,
            btn.y + (btn.height - labelSize.y) / 2
        };
        DrawTextEx(menuFont, buttonLabels[i], labelPos, fontSize, 2, WHITE);

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            TraceLog(LOG_INFO, "Button clicked: %s", buttonLabels[i]);
            switch (i) {
                case 0:
                    currentScreen = SCREEN_DIFFICULTY_MENU;
                    break;
                case 4:
                    currentScreen = SCREEN_EXIT;
                    break;
                default:
                    TraceLog(LOG_INFO, "Other options not implemented yet.");
                    break;
            }
        }
    }
}

void diffMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int totalHeight = DIFF_BUTTON_COUNT * buttonHeight + (DIFF_BUTTON_COUNT - 1) * buttonSpacing;
    int startY = (screenHeight - totalHeight) / 2;

    Vector2 mouse = GetMousePosition();

    const char *title = "DIFFICULTY";
    Vector2 titleSize = MeasureTextEx(titleFont, title, titleFontSize, 2);
    Vector2 titlePos = { (screenWidth - titleSize.x) / 2, 60 };
    DrawTextEx(titleFont, title, titlePos, titleFontSize, 2, WHITE);

    const char *backText = "BACK";
    Vector2 backSize = MeasureTextEx(menuFont, backText, fontSize, 2);
    Vector2 backPos = { 20, screenHeight - backSize.y - 20 };

    Rectangle backBounds = {
        backPos.x, backPos.y,
        backSize.x, backSize.y
    };

    bool backHover = CheckCollisionPointRec(mouse, backBounds);
    Color backColor = backHover ? RED : WHITE;

    DrawTextEx(menuFont, backText, backPos, fontSize, 2, backColor);

    if (backHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        TraceLog(LOG_INFO, "Back button clicked");
        currentScreen = SCREEN_MAIN_MENU;
    }
    for (int i = 0; i < DIFF_BUTTON_COUNT; i++) {
        Rectangle btn = {
            screenWidth / 2.0f - buttonWidth / 2.0f,
            startY + i * (buttonHeight + buttonSpacing),
            buttonWidth, buttonHeight
        };

        bool hovered = CheckCollisionPointRec(mouse, btn);
        Color btnColor = hovered ? LIGHTGRAY : DARKGRAY;
        DrawRectangleRec(btn, btnColor);

        Vector2 labelSize = MeasureTextEx(menuFont, diffLabels[i], fontSize, 2);
        Vector2 labelPos = {
            btn.x + (btn.width - labelSize.x) / 2,
            btn.y + (btn.height - labelSize.y) / 2
        };
        DrawTextEx(menuFont, diffLabels[i], labelPos, fontSize, 2, WHITE);

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            TraceLog(LOG_INFO, "Selected difficulty: %s", diffLabels[i]);
            selectedDifficulty = i; // Save selected difficulty
            currentScreen = SCREEN_GAMEMODE;
            // TODO: Move to game screen, or save selected difficulty
        }
    }
}

void gamemodeMenu(){
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int totalHeight = GAMEMODE_COUNT * buttonHeight + (GAMEMODE_COUNT - 1) * buttonSpacing;
    int startY = (screenHeight - totalHeight) / 2;

    Vector2 mouse = GetMousePosition();

    const char *title = "GAME MODE";
    Vector2 titleSize = MeasureTextEx(titleFont, title, titleFontSize, 2);
    Vector2 titlePos = { (screenWidth - titleSize.x) / 2, 60 };
    DrawTextEx(titleFont, title, titlePos, titleFontSize, 2, WHITE);

    const char *backText = "BACK";
    Vector2 backSize = MeasureTextEx(menuFont, backText, fontSize, 2);
    Vector2 backPos = { 20, screenHeight - backSize.y - 20 };

    Rectangle backBounds = {
        backPos.x, backPos.y,
        backSize.x, backSize.y
    };

    bool backHover = CheckCollisionPointRec(mouse, backBounds);
    Color backColor = backHover ? RED : WHITE;

    DrawTextEx(menuFont, backText, backPos, fontSize, 2, backColor);

    if (backHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        TraceLog(LOG_INFO, "Back button clicked");
        currentScreen = SCREEN_DIFFICULTY_MENU;
    }

    for (int i = 0; i < GAMEMODE_COUNT; i++) {
        Rectangle btn = {
            screenWidth / 2.0f - buttonWidth / 2.0f,
            startY + i * (buttonHeight + buttonSpacing),
            buttonWidth, buttonHeight
        };

        bool hovered = CheckCollisionPointRec(mouse, btn);
        Color btnColor = hovered ? LIGHTGRAY : DARKGRAY;
        DrawRectangleRec(btn, btnColor);

        Vector2 labelSize = MeasureTextEx(menuFont, gamemodeLabel[i], fontSize, 2);
        Vector2 labelPos = {
            btn.x + (btn.width - labelSize.x) / 2,
            btn.y + (btn.height - labelSize.y) / 2
        };
        DrawTextEx(menuFont, gamemodeLabel[i], labelPos, fontSize, 2, WHITE);

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            selectedGamemode = i; // Save selected gamemode
            TraceLog(LOG_INFO, "Selected gamemode: %s", gamemodeLabel[i]);
            currentScreen = SCREEN_GAME;    
            // TODO: Move to game screen, or save selected difficulty
        }


    }

}

