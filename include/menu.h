// menu.h
#ifndef MENU_H
#define MENU_H

#include <stdbool.h> // Include stdbool.h for bool type

typedef enum GameScreen {
    SCREEN_EXIT,
    SCREEN_MAIN_MENU,
    SCREEN_DIFFICULTY_MENU,
    SCREEN_GAMEMODE,
    SCREEN_GAME,
    SCREEN_SANDBOX,
    SCREEN_SETTINGS,
    SCREEN_CREDITS,
    SCREEN_STORY
} GameScreen;

extern GameScreen currentScreen;
extern int selectedDifficulty;
extern int selectedGamemode;

// Remove the isPaused declaration from here if it exists

extern bool isPaused;
void ParentMenu(void);
void diffMenu(void);
void gamemodeMenu(void);
void ingameMenu(void);

#endif
