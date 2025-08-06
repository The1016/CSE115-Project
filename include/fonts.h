// fonts.h
#ifndef FONTS_H
#define FONTS_H

#include "raylib.h"

// Declare fonts (accessible globally)
extern Font titleFont;
extern Font msgFont;
extern Font menuFont;

// Declare font loading/unloading functions
void LoadAllFonts(void);
void UnloadAllFonts(void);

#endif // FONTS_H
