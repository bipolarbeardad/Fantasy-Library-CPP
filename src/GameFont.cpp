#include "GameFont.h"

#include <string>


namespace
{
    Font gameFont;
    Font scriptFont;

    bool customFontLoaded =
        false;

    bool scriptFontLoaded =
        false;


    bool TryLoadFontInto(
        const char* path,
        Font& destination
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


        destination =
            candidate;


        SetTextureFilter(
            destination.texture,
            TEXTURE_FILTER_BILINEAR
        );


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


    Font ActiveScriptFont()
    {
        if (scriptFontLoaded)
        {
            return scriptFont;
        }


        return ActiveFont();
    }


    int MeasureWithFont(
        Font font,
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
                font,
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


    void DrawWithFont(
        Font font,
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
            font,
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
}


bool InitGameFont()
{
    // Main UI font.
    customFontLoaded =
        TryLoadFontInto(
            "C:/Windows/Fonts/segoeui.ttf",
            gameFont
        );


    if (!customFontLoaded)
    {
        customFontLoaded =
            TryLoadFontInto(
                "C:/Windows/Fonts/arial.ttf",
                gameFont
            );
    }


    // Script font costs the packaged game zero bytes because
    // it uses a normal Windows system font when available.
    scriptFontLoaded =
        TryLoadFontInto(
            "C:/Windows/Fonts/segoesc.ttf",
            scriptFont
        );


    if (!scriptFontLoaded)
    {
        scriptFontLoaded =
            TryLoadFontInto(
                "C:/Windows/Fonts/segoescb.ttf",
                scriptFont
            );
    }


    return customFontLoaded;
}


void ShutdownGameFont()
{
    if (scriptFontLoaded)
    {
        UnloadFont(
            scriptFont
        );


        scriptFontLoaded =
            false;
    }


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
    return MeasureWithFont(
        ActiveFont(),
        text,
        fontSize
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
    DrawWithFont(
        ActiveFont(),
        text,
        x,
        y,
        fontSize,
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


int MeasureScriptText(
    const char* text,
    int fontSize
)
{
    return MeasureWithFont(
        ActiveScriptFont(),
        text,
        fontSize
    );
}


int MeasureScriptText(
    const std::string& text,
    int fontSize
)
{
    return MeasureScriptText(
        text.c_str(),
        fontSize
    );
}


void DrawScriptText(
    const char* text,
    int x,
    int y,
    int fontSize,
    Color color
)
{
    DrawWithFont(
        ActiveScriptFont(),
        text,
        x,
        y,
        fontSize,
        color
    );
}


void DrawScriptText(
    const std::string& text,
    int x,
    int y,
    int fontSize,
    Color color
)
{
    DrawScriptText(
        text.c_str(),
        x,
        y,
        fontSize,
        color
    );
}