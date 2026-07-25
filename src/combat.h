#pragma once

#include <string>
#include <vector>

class Enemy;

class Combat
{
public:
    Combat();

    void HandleInput();

    Enemy* CheckAttack(
        std::vector<Enemy>& enemies
    );

    void ClearInput();

    const std::string& GetInput() const;

private:
    std::string currentInput;

    static std::string NormalizeWord(
        const std::string& word
    );
};