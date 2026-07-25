#include "GameFont.h"

#include <string>


namespace
{
    Font gameFont;

    bool customFontLoaded =
        false;


    bool TryLoadFont(
        const char* path
    )
    {
        if (!FileExists(path))
        {
            return false;
        }


        Font candidate =
            LoadFontEx(
                path,
                64,
                nullptr,
                0
            );


        if (
            candidate.texture.id == 0
            ||
            candidate.glyphCount <= 0
        )
        {
            return false;
        }


        gameFont =
            candidate;


        SetTextureFilter(
            gameFont.texture,
            TEXTURE_FILTER_BILINEAR
        );


        customFontLoaded =
            true;


        return true;
    }


    Font ActiveFont()
    {
        if (customFontLoaded)
        {
            return gameFont;
        }


        return GetFontDefault();
    }
}


bool InitGameFont()
{
    // Segoe UI is included with normal modern
    // Windows installations and costs the game
    // package zero additional bytes.
    if (
        TryLoadFont(
            "C:/Windows/Fonts/segoeui.ttf"
        )
    )
    {
        return true;
    }


    // Sensible Windows fallback.
    if (
        TryLoadFont(
            "C:/Windows/Fonts/arial.ttf"
        )
    )
    {
        return true;
    }


    // Raylib's built-in font remains a final
    // fallback so the game can still run.
    customFontLoaded =
        false;


    return false;
}


void ShutdownGameFont()
{
    if (customFontLoaded)
    {
        UnloadFont(
            gameFont
        );


        customFontLoaded =
            false;
    }
}


int MeasureGameText(
    const char* text,
    int fontSize
)
{
    if (
        text == nullptr
        ||
        text[0] == '\0'
    )
    {
        return 0;
    }


    const Vector2 size =
        MeasureTextEx(
            ActiveFont(),
            text,
            static_cast<float>(
                fontSize
            ),
            0.0f
        );


    return static_cast<int>(
        size.x + 0.5f
    );
}


int MeasureGameText(
    const std::string& text,
    int fontSize
)
{
    return MeasureGameText(
        text.c_str(),
        fontSize
    );
}


void DrawGameText(
    const char* text,
    int x,
    int y,
    int fontSize,
    Color color
)
{
    if (
        text == nullptr
        ||
        text[0] == '\0'
    )
    {
        return;
    }


    DrawTextEx(
        ActiveFont(),
        text,
        Vector2{
            static_cast<float>(x),
            static_cast<float>(y)
        },
        static_cast<float>(
            fontSize
        ),
        0.0f,
        color
    );
}


void DrawGameText(
    const std::string& text,
    int x,
    int y,
    int fontSize,
    Color color
)
{
    DrawGameText(
        text.c_str(),
        x,
        y,
        fontSize,
        color
    );
}