#include "MainMenu.h"
#include "GameFont.h"

#include "raylib.h"

#include <algorithm>


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
    // --------------------------------
    // MAIN MENU
    // --------------------------------

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


    // --------------------------------
    // OPTIONS
    // --------------------------------

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
    ClearBackground(
        Color{
            10,
            10,
            20,
            255
        }
    );


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


void MainMenu::DrawMain()
    const
{
    DrawCenteredText(
        "Fantasy Library",
        45,
        60,
        230,
        220,
        150
    );


    DrawCenteredText(
        "Quest of the Word Seeker",
        110,
        40,
        255,
        255,
        255
    );


    const char* description[] =
    {
        "Darkness has manifested in the realm.",
        "Only the true hero can help recover",
        "the words stolen by the shadow creatures.",
        "",
        "Fight your way past Goblins, Orcs,",
        "Beasts, and Legendary Creatures",
        "to restore The Tale of Bearly."
    };


    int y = 175;


    for (
        const char* line
        :
        description
    )
    {
        DrawCenteredText(
            line,
            y,
            28,
            200,
            200,
            200
        );


        y += 25;
    }


    const char* options[] =
    {
        "Start Adventure",
        "Read The Tale of Bearly",
        "Memories",
        "New Game",
        "Options",
        "Exit"
    };


    y =
        std::max(
            382,
            GetScreenHeight()
            -
            218
        );


    for (
        int index = 0;
        index < 6;
        index++
    )
    {
        const bool active =
            index == selected;


        DrawCenteredText(
            options[index],
            y,
            35,
            active ? 255 : 255,
            active ? 220 : 255,
            active ? 0 : 255
        );


        y += 36;
    }
}


void MainMenu::DrawOptions()
    const
{
    DrawCenteredText(
        "Options",
        85,
        60,
        230,
        220,
        150
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


    int y = 210;


    for (
        int index = 0;
        index < 3;
        index++
    )
    {
        const bool active =
            index == selected;


        DrawCenteredText(
            rows[index],
            y,
            32,
            active ? 255 : 235,
            active ? 220 : 235,
            active ? 0 : 235
        );


        y += 65;
    }


    DrawCenteredText(
        "W/S Select   A/D Adjust   Enter Confirm   ESC Back",
        GetScreenHeight() - 45,
        24,
        155,
        155,
        165
    );
}