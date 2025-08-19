#ifndef MENU_H
#define MENU_H

typedef enum GameScreen {
    SCREEN_MAIN_MENU,
    SCREEN_DIFFICULTY_MENU,
    SCREEN_GAMEMODE,
    SCREEN_GAME,
    SCREEN_SANDBOX,
    SCREEN_STORY,      // added for Story Mode
    SCREEN_SETTINGS,
    SCREEN_CREDITS,
    SCREEN_EXIT
} GameScreen;

// Shared screen state
extern GameScreen currentScreen;

// Selected options
extern int selectedDifficulty;
extern int selectedGamemode;

// Menu functions
void ParentMenu(void);
void diffMenu(void);
void gamemodeMenu(void);

// Story Mode function
void storyMode(void);

#endif // MENU_H
