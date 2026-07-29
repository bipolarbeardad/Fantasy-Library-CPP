#include "EnemySprites.h"

#include "Enemy.h"

#include <algorithm>


Texture2D EnemySprites::texture = {};

bool EnemySprites::loaded = false;


bool EnemySprites::Load()
{
    if (loaded)
    {
        return true;
    }


    const char* path =
        "assets/enemy_sprites.png";


    if (!FileExists(path))
    {
        TraceLog(
            LOG_WARNING,
            "Enemy sprite sheet not found: %s",
            path
        );

        return false;
    }


    texture =
        LoadTexture(
            path
        );


    if (texture.id == 0)
    {
        TraceLog(
            LOG_WARNING,
            "Failed to load enemy sprite sheet."
        );

        return false;
    }


    // Important for pixel art.
    SetTextureFilter(
        texture,
        TEXTURE_FILTER_POINT
    );


    loaded = true;


    return true;
}


void EnemySprites::Unload()
{
    if (!loaded)
    {
        return;
    }


    UnloadTexture(
        texture
    );


    texture = {};


    loaded = false;
}


bool EnemySprites::IsLoaded()
{
    return loaded;
}


Texture2D EnemySprites::GetTexture()
{
    return texture;
}


Rectangle EnemySprites::GetFrame(
    EnemyType type,
    int animationFrame
)
{
    constexpr int FRAME_SIZE =
        32;


    constexpr int FRAME_COUNT =
        3;


    animationFrame =
        std::clamp(
            animationFrame,
            0,
            FRAME_COUNT - 1
        );


    int row = 0;


    switch (type)
    {
        case EnemyType::Goblin:
            row = 0;
            break;


        case EnemyType::Orc:
            row = 1;
            break;


        case EnemyType::Wolf:
            row = 2;
            break;


        case EnemyType::Bat:
            row = 3;
            break;


        case EnemyType::Dragon:
            row = 4;
            break;
    }


    return Rectangle{
        static_cast<float>(
            animationFrame
            *
            FRAME_SIZE
        ),

        static_cast<float>(
            row
            *
            FRAME_SIZE
        ),

        static_cast<float>(
            FRAME_SIZE
        ),

        static_cast<float>(
            FRAME_SIZE
        )
    };
}