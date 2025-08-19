#include "raylib.h"
#include "fonts.h"

Font titleFont;
Font msgFont;
Font menuFont;

void LoadAllFonts(void)
{
    titleFont = LoadFontEx("assets/CinzelDecorative-Regular.ttf", 140, 0, 0);
    GenTextureMipmaps(&titleFont.texture);
    SetTextureFilter(titleFont.texture, TEXTURE_FILTER_TRILINEAR);

    msgFont = LoadFontEx("assets/CinzelDecorative-Regular.ttf", 30, 0, 0);
    GenTextureMipmaps(&msgFont.texture);
    SetTextureFilter(msgFont.texture, TEXTURE_FILTER_TRILINEAR);

    menuFont = LoadFontEx("assets/CinzelDecorative-Regular.ttf", 50, 0, 0);
    GenTextureMipmaps(&menuFont.texture);
    SetTextureFilter(menuFont.texture, TEXTURE_FILTER_TRILINEAR);

    if (titleFont.texture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load title font!");
        CloseWindow();
    }
    if (msgFont.texture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load message font!");
        CloseWindow();
    }
    if (menuFont.texture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load menu font!");
        CloseWindow();
    }
}

void UnloadAllFonts(void)
{
    UnloadFont(titleFont);
    UnloadFont(msgFont);
    UnloadFont(menuFont);
}
