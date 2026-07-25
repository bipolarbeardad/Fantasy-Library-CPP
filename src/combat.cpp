#include "Combat.h"
#include "Enemy.h"
#include "raylib.h"

#include <cctype>


Combat::Combat()
{
    currentInput = "";
}


void Combat::HandleInput()
{
    // Backspace removes one character.

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        if (!currentInput.empty())
        {
            currentInput.pop_back();
        }
    }


    // Enter clears everything typed.

    if (IsKeyPressed(KEY_ENTER))
    {
        currentInput.clear();
        return;
    }


    // Read typed characters.

    int key = GetCharPressed();

    while (key > 0)
    {
        if (
            key >= 'A' &&
            key <= 'Z'
        )
        {
            key = std::tolower(key);
        }


        if (
            key >= 'a' &&
            key <= 'z'
        )
        {
            currentInput +=
                static_cast<char>(key);
        }


        key = GetCharPressed();
    }
}


Enemy* Combat::CheckAttack(
    std::vector<Enemy>& enemies
)
{
    if (currentInput.empty())
    {
        return nullptr;
    }


    std::string typed =
        NormalizeWord(currentInput);


    for (Enemy& enemy : enemies)
    {
        if (enemy.IsDefeated())
        {
            continue;
        }


        if (enemy.HasEscaped())
        {
            continue;
        }


        std::string target =
            NormalizeWord(
                enemy.GetWordDisplay()
            );


        if (typed == target)
        {
            currentInput.clear();

            return &enemy;
        }
    }


    return nullptr;
}


void Combat::ClearInput()
{
    currentInput.clear();
}


const std::string&
Combat::GetInput() const
{
    return currentInput;
}


std::string Combat::NormalizeWord(
    const std::string& word
)
{
    std::string result;


    for (unsigned char character : word)
    {
        if (
            character >= 'A' &&
            character <= 'Z'
        )
        {
            result +=
                static_cast<char>(
                    std::tolower(character)
                );
        }

        else if (
            character >= 'a' &&
            character <= 'z'
        )
        {
            result +=
                static_cast<char>(
                    character
                );
        }

        // Everything else is ignored.
        // This means apostrophes do not
        // need to be typed.
    }


    return result;
}