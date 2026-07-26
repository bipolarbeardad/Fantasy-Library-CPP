#include "MainMenu.h"
#include "GameFont.h"

#include "raylib.h"

#include <algorithm>
#include <cmath>


MainMenu::MainMenu()
    :
    mode(Mode::Main),
    selected(0),
    displayLabel("900 x 600"),
    musicVolume(40)
{
}


void MainMenu::SetSettings(
    const std::string& newDisplayLabel,
    int newMusicVolume
)
{
    displayLabel =
        newDisplayLabel;


    musicVolume =
        std::clamp(
            newMusicVolume,
            0,
            100
        );
}


void MainMenu::MoveSelection(
    int amount,
    int count
)
{
    selected =
        (
            selected
            +
            amount
            +
            count
        )
        %
        count;
}


MenuAction MainMenu::HandleInput()
{
    if (mode == Mode::Main)
    {
        if (
            IsKeyPressed(KEY_UP)
            ||
            IsKeyPressed(KEY_W)
        )
        {
            MoveSelection(
                -1,
                6
            );
        }


        else if (
            IsKeyPressed(KEY_DOWN)
            ||
            IsKeyPressed(KEY_S)
        )
        {
            MoveSelection(
                1,
                6
            );
        }


        else if (
            IsKeyPressed(KEY_ENTER)
        )
        {
            switch (selected)
            {
                case 0:
                    return MenuAction::Start;

                case 1:
                    return MenuAction::Story;

                case 2:
                    return MenuAction::Memories;

                case 3:
                    return MenuAction::NewGame;

                case 4:
                    mode = Mode::Options;
                    selected = 0;
                    return MenuAction::None;

                case 5:
                    return MenuAction::Exit;
            }
        }


        return MenuAction::None;
    }


    if (IsKeyPressed(KEY_ESCAPE))
    {
        mode = Mode::Main;
        selected = 3;

        return MenuAction::None;
    }


    if (
        IsKeyPressed(KEY_UP)
        ||
        IsKeyPressed(KEY_W)
    )
    {
        MoveSelection(
            -1,
            3
        );
    }


    else if (
        IsKeyPressed(KEY_DOWN)
        ||
        IsKeyPressed(KEY_S)
    )
    {
        MoveSelection(
            1,
            3
        );
    }


    else if (
        IsKeyPressed(KEY_LEFT)
        ||
        IsKeyPressed(KEY_A)
    )
    {
        if (selected == 0)
        {
            return MenuAction::DisplayLeft;
        }


        if (selected == 1)
        {
            return MenuAction::VolumeDown;
        }
    }


    else if (
        IsKeyPressed(KEY_RIGHT)
        ||
        IsKeyPressed(KEY_D)
    )
    {
        if (selected == 0)
        {
            return MenuAction::DisplayRight;
        }


        if (selected == 1)
        {
            return MenuAction::VolumeUp;
        }
    }


    else if (
        IsKeyPressed(KEY_ENTER)
        &&
        selected == 2
    )
    {
        mode = Mode::Main;
        selected = 3;
    }


    return MenuAction::None;
}


void MainMenu::Draw()
    const
{
    DrawTableBackground();


    if (mode == Mode::Main)
    {
        DrawMain();
    }

    else
    {
        DrawOptions();
    }
}


void MainMenu::DrawCenteredText(
    const std::string& text,
    int y,
    int fontSize,
    unsigned char r,
    unsigned char g,
    unsigned char b
)
{
    const int width =
        MeasureGameText(
            text.c_str(),
            fontSize
        );


    DrawGameText(
        text.c_str(),
        GetScreenWidth() / 2
        -
        width / 2,
        y,
        fontSize,
        Color{
            r,
            g,
            b,
            255
        }
    );
}


void MainMenu::DrawCenteredScriptText(
    const std::string& text,
    int y,
    int fontSize,
    unsigned char r,
    unsigned char g,
    unsigned char b
)
{
    const int width =
        MeasureScriptText(
            text.c_str(),
            fontSize
        );


    DrawScriptText(
        text.c_str(),
        GetScreenWidth() / 2
        -
        width / 2,
        y,
        fontSize,
        Color{
            r,
            g,
            b,
            255
        }
    );
}


void MainMenu::DrawTableBackground()
    const
{
    const int width =
        GetScreenWidth();


    const int height =
        GetScreenHeight();


    ClearBackground(
        Color{
            55,
            35,
            24,
            255
        }
    );


    // Wooden tabletop bands.
    const int plankHeight =
        std::max(
            48,
            height / 10
        );


    for (
        int y = 0;
        y < height;
        y += plankHeight
    )
    {
        const bool alternate =
            (
                y / plankHeight
            )
            %
            2
            ==
            0;


        DrawRectangle(
            0,
            y,
            width,
            plankHeight,
            alternate
            ?
            Color{
                72,
                46,
                30,
                255
            }
            :
            Color{
                64,
                40,
                27,
                255
            }
        );


        DrawLine(
            0,
            y,
            width,
            y,
            Color{
                42,
                27,
                20,
                255
            }
        );


        // A few subtle grain marks.
        for (
            int index = 0;
            index < 7;
            index++
        )
        {
            const int grainY =
                y
                +
                8
                +
                index * 5;


            DrawLine(
                0,
                grainY,
                width,
                grainY,
                Color{
                    84,
                    55,
                    35,
                    55
                }
            );
        }
    }
}


void MainMenu::DrawRibbon(
    int bookX,
    int bookY,
    int bookWidth
)
    const
{
    const int ribbonX =
        bookX
        +
        static_cast<int>(
            bookWidth * 0.18f
        );


    const int ribbonWidth =
        std::max(
            18,
            bookWidth / 24
        );


    DrawRectangle(
        ribbonX,
        std::max(
            0,
            bookY - 34
        ),
        ribbonWidth,
        52,
        Color{
            65,
            92,
            165,
            255
        }
    );


    DrawRectangle(
        ribbonX + 3,
        std::max(
            0,
            bookY - 34
        ),
        std::max(
            3,
            ribbonWidth / 5
        ),
        52,
        Color{
            92,
            120,
            195,
            180
        }
    );


    // Split ribbon tip.
    DrawTriangle(
        Vector2{
            static_cast<float>(
                ribbonX
            ),
            static_cast<float>(
                bookY + 18
            )
        },
        Vector2{
            static_cast<float>(
                ribbonX
                +
                ribbonWidth / 2
            ),
            static_cast<float>(
                bookY + 29
            )
        },
        Vector2{
            static_cast<float>(
                ribbonX
                +
                ribbonWidth
            ),
            static_cast<float>(
                bookY + 18
            )
        },
        Color{
            65,
            92,
            165,
            255
        }
    );
}


void MainMenu::DrawBookDecorations(
    int bookX,
    int bookY,
    int bookWidth,
    int bookHeight
)
    const
{
    const Color gold =
    {
        205,
        172,
        82,
        255
    };


    const Color goldDark =
    {
        135,
        102,
        45,
        255
    };


    const int inset =
        std::max(
            18,
            bookWidth / 28
        );


    const Rectangle outer =
    {
        static_cast<float>(
            bookX + inset
        ),
        static_cast<float>(
            bookY + inset
        ),
        static_cast<float>(
            bookWidth - inset * 2
        ),
        static_cast<float>(
            bookHeight - inset * 2
        )
    };


    const Rectangle inner =
    {
        outer.x + 9.0f,
        outer.y + 9.0f,
        outer.width - 18.0f,
        outer.height - 18.0f
    };


    DrawRectangleLinesEx(
        outer,
        3.0f,
        goldDark
    );


    DrawRectangleLinesEx(
        inner,
        1.5f,
        gold
    );


    const float curl =
        static_cast<float>(
            std::max(
                15,
                bookWidth / 30
            )
        );


    // Decorative corner curls.
    const Vector2 corners[] =
    {
        {
            inner.x,
            inner.y
        },
        {
            inner.x + inner.width,
            inner.y
        },
        {
            inner.x,
            inner.y + inner.height
        },
        {
            inner.x + inner.width,
            inner.y + inner.height
        }
    };


    for (
        int index = 0;
        index < 4;
        index++
    )
    {
        const float xDirection =
            (
                index == 0
                ||
                index == 2
            )
            ?
            1.0f
            :
            -1.0f;


        const float yDirection =
            (
                index < 2
            )
            ?
            1.0f
            :
            -1.0f;


        const Vector2 corner =
            corners[index];


        DrawLineEx(
            corner,
            Vector2{
                corner.x
                +
                curl * xDirection,
                corner.y
            },
            2.0f,
            gold
        );


        DrawLineEx(
            corner,
            Vector2{
                corner.x,
                corner.y
                +
                curl * yDirection
            },
            2.0f,
            gold
        );


        DrawCircleLines(
            static_cast<int>(
                corner.x
                +
                curl * 0.55f * xDirection
            ),
            static_cast<int>(
                corner.y
                +
                curl * 0.55f * yDirection
            ),
            curl * 0.45f,
            goldDark
        );
    }


    // Small central flourish near the bottom.
    const float centerX =
        static_cast<float>(
            bookX
            +
            bookWidth / 2
        );


    const float flourishY =
        static_cast<float>(
            bookY
            +
            bookHeight
            -
            inset
            -
            26
        );


    DrawTriangle(
        Vector2{
            centerX,
            flourishY - 8.0f
        },
        Vector2{
            centerX - 12.0f,
            flourishY + 6.0f
        },
        Vector2{
            centerX + 12.0f,
            flourishY + 6.0f
        },
        gold
    );


    DrawCircleLines(
        static_cast<int>(
            centerX
        ),
        static_cast<int>(
            flourishY - 2.0f
        ),
        9.0f,
        goldDark
    );
}


void MainMenu::DrawBookCover()
    const
{
    const int screenWidth =
        GetScreenWidth();


    const int screenHeight =
        GetScreenHeight();


    const int maxBookWidth =
        static_cast<int>(
            screenWidth * 0.62f
        );


    const int maxBookHeight =
        static_cast<int>(
            screenHeight * 0.90f
        );


    const float targetAspect =
        0.72f;


    int bookHeight =
        maxBookHeight;


    int bookWidth =
        static_cast<int>(
            bookHeight
            *
            targetAspect
        );


    if (bookWidth > maxBookWidth)
    {
        bookWidth =
            maxBookWidth;


        bookHeight =
            static_cast<int>(
                bookWidth
                /
                targetAspect
            );
    }


    const int bookX =
        screenWidth / 2
        -
        bookWidth / 2;


    const int bookY =
        screenHeight / 2
        -
        bookHeight / 2;


    // Shadow.
    DrawRectangleRounded(
        Rectangle{
            static_cast<float>(
                bookX + 12
            ),
            static_cast<float>(
                bookY + 14
            ),
            static_cast<float>(
                bookWidth
            ),
            static_cast<float>(
                bookHeight
            )
        },
        0.03f,
        10,
        Color{
            15,
            10,
            10,
            150
        }
    );


    DrawRibbon(
        bookX,
        bookY,
        bookWidth
    );


    // Main purple leather cover.
    DrawRectangleRounded(
        Rectangle{
            static_cast<float>(
                bookX
            ),
            static_cast<float>(
                bookY
            ),
            static_cast<float>(
                bookWidth
            ),
            static_cast<float>(
                bookHeight
            )
        },
        0.025f,
        10,
        Color{
            74,
            34,
            78,
            255
        }
    );


    // Spine.
    DrawRectangle(
        bookX,
        bookY + 4,
        std::max(
            16,
            bookWidth / 18
        ),
        bookHeight - 8,
        Color{
            55,
            23,
            60,
            255
        }
    );


    DrawLine(
        bookX
        +
        std::max(
            16,
            bookWidth / 18
        )
        +
        5,
        bookY + 8,
        bookX
        +
        std::max(
            16,
            bookWidth / 18
        )
        +
        5,
        bookY
        +
        bookHeight
        -
        8,
        Color{
            98,
            55,
            100,
            255
        }
    );


    // Very subtle leather grain.
    for (
        int index = 0;
        index < 18;
        index++
    )
    {
        const int y =
            bookY
            +
            15
            +
            index
            *
            std::max(
                6,
                bookHeight / 22
            );


        DrawLine(
            bookX + 12,
            y,
            bookX + bookWidth - 12,
            y,
            Color{
                105,
                62,
                108,
                28
            }
        );
    }


    DrawBookDecorations(
        bookX,
        bookY,
        bookWidth,
        bookHeight
    );
}


void MainMenu::DrawMain()
    const
{
    DrawBookCover();


    const int screenHeight =
        GetScreenHeight();


    // Script title, as though embossed onto the cover.
    DrawCenteredScriptText(
        "Fantasy Library",
        static_cast<int>(
            screenHeight * 0.12f
        ),
        std::max(
            42,
            screenHeight / 11
        ),
        230,
        197,
        95
    );


    DrawCenteredText(
        "Quest of the Word Seeker",
        static_cast<int>(
            screenHeight * 0.24f
        ),
        std::max(
            22,
            screenHeight / 27
        ),
        220,
        205,
        155
    );


    DrawCenteredScriptText(
        "The Tale of Bearly",
        static_cast<int>(
            screenHeight * 0.31f
        ),
        std::max(
            25,
            screenHeight / 24
        ),
        195,
        165,
        90
    );


    const char* options[] =
    {
        "Start Adventure",
        "Read The Tale of Bearly",
        "Memories",
        "New Game",
        "Options",
        "Exit"
    };


    int y =
        static_cast<int>(
            screenHeight * 0.43f
        );


    const int spacing =
        std::max(
            35,
            screenHeight / 14
        );


    const int fontSize =
        std::max(
            23,
            screenHeight / 25
        );


    for (
        int index = 0;
        index < 6;
        index++
    )
    {
        const bool active =
            index
            ==
            selected;


        if (active)
        {
            const int textWidth =
                MeasureGameText(
                    options[index],
                    fontSize
                );


            DrawRectangleRounded(
                Rectangle{
                    static_cast<float>(
                        GetScreenWidth() / 2
                        -
                        textWidth / 2
                        -
                        16
                    ),
                    static_cast<float>(
                        y - 4
                    ),
                    static_cast<float>(
                        textWidth + 32
                    ),
                    static_cast<float>(
                        fontSize + 10
                    )
                },
                0.25f,
                8,
                Color{
                    205,
                    170,
                    75,
                    32
                }
            );
        }


        DrawCenteredText(
            options[index],
            y,
            fontSize,
            active ? 255 : 210,
            active ? 220 : 190,
            active ? 100 : 145
        );


        y += spacing;
    }
}


void MainMenu::DrawOptions()
    const
{
    DrawBookCover();


    const int screenHeight =
        GetScreenHeight();


    DrawCenteredScriptText(
        "Options",
        static_cast<int>(
            screenHeight * 0.15f
        ),
        std::max(
            44,
            screenHeight / 11
        ),
        230,
        197,
        95
    );


    const std::string rows[] =
    {
        "Screen Size    < "
        +
        displayLabel
        +
        " >",

        "Music Volume    < "
        +
        std::to_string(
            musicVolume
        )
        +
        "% >",

        "Back"
    };


    int y =
        static_cast<int>(
            screenHeight * 0.38f
        );


    const int spacing =
        std::max(
            58,
            screenHeight / 8
        );


    const int fontSize =
        std::max(
            24,
            screenHeight / 24
        );


    for (
        int index = 0;
        index < 3;
        index++
    )
    {
        const bool active =
            index
            ==
            selected;


        DrawCenteredText(
            rows[index],
            y,
            fontSize,
            active ? 255 : 210,
            active ? 220 : 190,
            active ? 100 : 145
        );


        y += spacing;
    }


    DrawCenteredText(
        "W/S Select   A/D Adjust   Enter Confirm   ESC Back",
        GetScreenHeight() - 42,
        18,
        185,
        170,
        135
    );
}