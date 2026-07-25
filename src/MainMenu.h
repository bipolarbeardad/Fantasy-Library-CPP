#pragma once

#include <string>


enum class MenuAction
{
    None,
    Start,
    Story,
    NewGame,
    Exit,
    DisplayLeft,
    DisplayRight,
    VolumeDown,
    VolumeUp
};


class MainMenu
{
public:

    MainMenu();


    MenuAction HandleInput();

    void Draw() const;


    void SetSettings(
        const std::string& displayLabel,
        int musicVolume
    );


private:

    enum class Mode
    {
        Main,
        Options
    };


    Mode mode;

    int selected;

    std::string displayLabel;

    int musicVolume;


    void MoveSelection(
        int amount,
        int count
    );


    void DrawMain() const;

    void DrawOptions() const;


    static void DrawCenteredText(
        const std::string& text,
        int y,
        int fontSize,
        unsigned char r,
        unsigned char g,
        unsigned char b
    );
};