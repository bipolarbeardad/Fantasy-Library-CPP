#pragma once

#include "raylib.h"

enum class EnemyType;

class EnemySprites
{
public:
    static bool Load();

    static void Unload();

    static bool IsLoaded();

    static Texture2D GetTexture();

    static Rectangle GetFrame(
        EnemyType type,
        int animationFrame
    );

private:
    static Texture2D texture;

    static bool loaded;
};