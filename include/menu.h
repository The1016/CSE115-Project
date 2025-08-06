// menu.h
#ifndef MENU_H
#define MENU_H

typedef enum GameScreen {
    SCREEN_MAIN_MENU,
    SCREEN_DIFFICULTY_MENU,
    SCREEN_GAMEMODE,
    SCREEN_GAME,
    SCREEN_SETTINGS,
    SCREEN_CREDITS,
    SCREEN_EXIT
} GameScreen;

// Shared screen state
extern GameScreen currentScreen;

void ParentMenu(void);
void diffMenu(void);

#endif
