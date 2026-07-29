#include "Enemy.h"

#include "EnemySprites.h"
#include "GameFont.h"

#include <algorithm>
#include <cmath>


namespace
{
    constexpr float PI_F =
        3.14159265358979323846f;
}


Enemy::Enemy(
    const std::vector<std::string>& words,
    const std::vector<int>& wordIds,
    EnemyType enemyType,
    int lane,
    float startX,
    float startY,
    float targetX,
    float targetY,
    EnemySpeed speedType
)
    :
    words(words),
    wordIds(wordIds),
    enemyType(enemyType),
    lane(lane),
    x(startX),
    y(startY),
    targetX(targetX),
    targetY(targetY),
    speedType(speedType),
    speed(
        SpeedValue(speedType)
    ),
    currentWordIndex(0),
    defeated(false),
    defeatTimer(0),
    escaped(false),
    stunned(false),
    stunTimer(0),
    movementStunTimer(0),
    walkPhase(
        static_cast<float>(
            GetRandomValue(
                0,
                6283
            )
        ) / 1000.0f
    )
{
}


float Enemy::SpeedValue(
    EnemySpeed type
)
{
    switch (type)
    {
        case EnemySpeed::Slow:
            return 0.60f;

        case EnemySpeed::Fast:
            return 1.25f;

        default:
            return 0.90f;
    }
}


EnemySpeed Enemy::RandomSpeed()
{
    const int roll =
        GetRandomValue(
            1,
            100
        );


    if (roll <= 30)
    {
        return EnemySpeed::Slow;
    }


    if (roll <= 75)
    {
        return EnemySpeed::Medium;
    }


    return EnemySpeed::Fast;
}


const std::string&
Enemy::GetCurrentWord() const
{
    static const std::string empty;


    if (
        currentWordIndex
        >=
        static_cast<int>(
            words.size()
        )
    )
    {
        return empty;
    }


    return words[
        currentWordIndex
    ];
}


int Enemy::GetCurrentWordId() const
{
    if (
        currentWordIndex
        >=
        static_cast<int>(
            wordIds.size()
        )
    )
    {
        return -1;
    }


    return wordIds[
        currentWordIndex
    ];
}


std::string Enemy::GetWordDisplay() const
{
    if (stunned)
    {
        return "";
    }


    return GetCurrentWord();
}


int Enemy::GetWordsRemaining() const
{
    return std::max(
        0,
        static_cast<int>(
            words.size()
        )
        -
        currentWordIndex
    );
}


std::vector<int>
Enemy::GetRemainingWordIds() const
{
    if (
        currentWordIndex
        >=
        static_cast<int>(
            wordIds.size()
        )
    )
    {
        return {};
    }


    return std::vector<int>(
        wordIds.begin()
        +
        currentWordIndex,
        wordIds.end()
    );
}


int Enemy::GetStarCount() const
{
    return std::max(
        0,
        GetWordsRemaining() - 1
    );
}


bool Enemy::CompleteCurrentWord()
{
    if (
        defeated
        ||
        escaped
    )
    {
        return false;
    }


    currentWordIndex++;


    if (
        currentWordIndex
        >=
        static_cast<int>(
            words.size()
        )
    )
    {
        return true;
    }


    stunned = true;

    stunTimer =
        STUN_DURATION;


    return false;
}


void Enemy::Update()
{
    if (
        defeated
        ||
        escaped
    )
    {
        return;
    }


    // The short between-word stagger hides the next word and
    // temporarily prevents movement.
    if (stunned)
    {
        stunTimer--;


        if (stunTimer <= 0)
        {
            stunTimer = 0;

            stunned = false;
        }


        // Quill stun still counts down while the enemy is in its
        // normal post-hit stagger, so the timers do not stack in
        // a confusing way.
        if (movementStunTimer > 0)
        {
            movementStunTimer--;
        }


        return;
    }


    // Quill Stun and Freeze pause movement but deliberately leave
    // the current word visible and typeable.
    if (movementStunTimer > 0)
    {
        movementStunTimer--;

        return;
    }


    // Lower lanes begin converging toward the Word Seeker
    // earlier so their sprites and labels pass above the Quill HUD.
    // Upper lanes keep a later merge because they do not intersect it.
    float convergeDistance =
        95.0f;


    if (lane == 3)
    {
        convergeDistance =
            185.0f;
    }

    else if (lane == 4)
    {
        convergeDistance =
            285.0f;
    }


    const float convergeX =
        targetX
        +
        convergeDistance;


    if (x > convergeX)
    {
        x -= speed;
    }

    else
    {
        const float dx =
            targetX - x;


        const float dy =
            targetY - y;


        const float distance =
            std::sqrt(
                dx * dx
                +
                dy * dy
            );


        if (distance > 0.0f)
        {
            x +=
                (
                    dx / distance
                )
                *
                speed;


            y +=
                (
                    dy / distance
                )
                *
                speed;
        }
    }


    // Advance the sprite animation independently of movement speed.
    // At 60 FPS, 0.0266667 per update gives the four-step
    // 0 -> 1 -> 2 -> 1 cycle a total duration of about 2.5 seconds.
    walkPhase +=
        0.0266667f;
}


void Enemy::ApplyStun(
    int frames
)
{
    if (
        defeated
        ||
        escaped
        ||
        frames <= 0
    )
    {
        return;
    }


    // Never shorten an existing Quill/Freeze stun.
    movementStunTimer =
        std::max(
            movementStunTimer,
            frames
        );
}


bool Enemy::IsMovementStunned() const
{
    return movementStunTimer > 0;
}


int Enemy::GetStunFramesRemaining() const
{
    return movementStunTimer;
}


bool Enemy::HasReachedTarget() const
{
    const float dx =
        targetX - x;


    const float dy =
        targetY - y;


    return
        std::sqrt(
            dx * dx
            +
            dy * dy
        )
        <=
        32.0f;
}


void Enemy::Defeat()
{
    defeated = true;

    defeatTimer = 45;
}


void Enemy::UpdateDefeat()
{
    if (defeatTimer > 0)
    {
        defeatTimer--;
    }
}


int Enemy::GetAnimationFrame() const
{
    // Four animation phases:
    //
    // 0 -> 1 -> 2 -> 1
    //
    // walkPhase advances by 0.0266667 each game update.
    // At 60 FPS that produces one complete cycle in
    // approximately 2.5 seconds.
    const int phase =
        static_cast<int>(
            walkPhase
        )
        &
        3;


    switch (phase)
    {
        case 0:
            return 0;

        case 1:
            return 1;

        case 2:
            return 2;

        default:
            return 1;
    }
}


EnemyType Enemy::GetSpriteType() const
{
    // Until WordManager is updated, old Beast spawns use the Wolf
    // row so the current game remains compatible.
    if (enemyType == EnemyType::Beast)
    {
        return EnemyType::Wolf;
    }


    return enemyType;
}


float Enemy::GetSpriteDrawSize() const
{
    switch (GetSpriteType())
    {
        case EnemyType::Goblin:
            return 54.0f;

        case EnemyType::Orc:
            return 68.0f;

        case EnemyType::Wolf:
            return 64.0f;

        case EnemyType::Bat:
            return 60.0f;

        case EnemyType::Dragon:
            return 78.0f;

        default:
            return 62.0f;
    }
}


float Enemy::GetSpriteYOffset() const
{
    switch (GetSpriteType())
    {
        case EnemyType::Bat:
            return -10.0f;

        case EnemyType::Dragon:
            return -5.0f;

        default:
            return 0.0f;
    }
}


void Enemy::DrawSpriteEnemy(
    float drawX,
    float drawY
) const
{
    if (!EnemySprites::IsLoaded())
    {
        switch (GetSpriteType())
        {
            case EnemyType::Goblin:
                DrawGoblin(
                    drawX,
                    drawY
                );
                return;

            case EnemyType::Orc:
                DrawOrc(
                    drawX,
                    drawY
                );
                return;

            default:
                DrawBeast(
                    drawX,
                    drawY
                );
                return;
        }
    }


    const EnemyType spriteType =
        GetSpriteType();


    const Rectangle source =
        EnemySprites::GetFrame(
            spriteType,
            GetAnimationFrame()
        );


    const float drawSize =
        GetSpriteDrawSize();


    float spriteY =
        drawY
        +
        GetSpriteYOffset();


    // Small airborne bob. It pauses naturally during Quill
    // stun/freeze because walkPhase also pauses.
    if (
        spriteType == EnemyType::Bat
        ||
        spriteType == EnemyType::Dragon
    )
    {
        spriteY +=
            std::sin(
                walkPhase * 0.8f
            )
            *
            3.0f;
    }


    const Rectangle destination =
    {
        drawX,
        spriteY,
        drawSize,
        drawSize
    };


    const Vector2 origin =
    {
        drawSize / 2.0f,
        drawSize / 2.0f
    };


    DrawTexturePro(
        EnemySprites::GetTexture(),
        source,
        destination,
        origin,
        0.0f,
        WHITE
    );
}


void Enemy::DrawGoblin(
    float drawX,
    float drawY
) const
{
    const Color skin =
    {
        65,
        165,
        75,
        255
    };


    const Color dark =
    {
        25,
        70,
        35,
        255
    };


    const Color cloth =
    {
        95,
        70,
        55,
        255
    };


    drawY +=
        std::sin(
            walkPhase
        )
        *
        2.0f;


    DrawEllipse(
        static_cast<int>(drawX),
        static_cast<int>(
            drawY + 12
        ),
        13,
        16,
        cloth
    );


    DrawCircle(
        static_cast<int>(drawX),
        static_cast<int>(
            drawY - 16
        ),
        15,
        skin
    );


    DrawTriangle(
        Vector2{
            drawX - 12,
            drawY - 20
        },
        Vector2{
            drawX - 30,
            drawY - 25
        },
        Vector2{
            drawX - 13,
            drawY - 10
        },
        skin
    );


    DrawTriangle(
        Vector2{
            drawX + 12,
            drawY - 20
        },
        Vector2{
            drawX + 13,
            drawY - 10
        },
        Vector2{
            drawX + 30,
            drawY - 25
        },
        skin
    );


    const Color eye =
    {
        245,
        235,
        150,
        255
    };


    DrawCircle(
        static_cast<int>(
            drawX - 5
        ),
        static_cast<int>(
            drawY - 18
        ),
        2,
        eye
    );


    DrawCircle(
        static_cast<int>(
            drawX + 5
        ),
        static_cast<int>(
            drawY - 18
        ),
        2,
        eye
    );


    const float legOffset =
        std::sin(
            walkPhase
        )
        *
        4.0f;


    DrawLineEx(
        Vector2{
            drawX - 7,
            drawY + 24
        },
        Vector2{
            drawX - 10
            +
            legOffset,
            drawY + 35
        },
        4,
        dark
    );


    DrawLineEx(
        Vector2{
            drawX + 7,
            drawY + 24
        },
        Vector2{
            drawX + 10
            -
            legOffset,
            drawY + 35
        },
        4,
        dark
    );
}


void Enemy::DrawOrc(
    float drawX,
    float drawY
) const
{
    const Color skin =
    {
        105,
        125,
        65,
        255
    };


    const Color armor =
    {
        85,
        80,
        70,
        255
    };


    const Color tusk =
    {
        235,
        225,
        190,
        255
    };


    drawY +=
        std::sin(
            walkPhase
        )
        *
        1.5f;


    DrawRectangleRounded(
        Rectangle{
            drawX - 22,
            drawY - 5,
            44,
            38
        },
        0.2f,
        4,
        armor
    );


    DrawRectangleRounded(
        Rectangle{
            drawX - 17,
            drawY - 31,
            34,
            29
        },
        0.25f,
        4,
        skin
    );


    DrawLineEx(
        Vector2{
            drawX - 11,
            drawY - 21
        },
        Vector2{
            drawX + 11,
            drawY - 21
        },
        4,
        Color{
            40,
            45,
            30,
            255
        }
    );


    const Color eye =
    {
        230,
        190,
        80,
        255
    };


    DrawCircle(
        static_cast<int>(
            drawX - 7
        ),
        static_cast<int>(
            drawY - 17
        ),
        2,
        eye
    );


    DrawCircle(
        static_cast<int>(
            drawX + 7
        ),
        static_cast<int>(
            drawY - 17
        ),
        2,
        eye
    );


    DrawTriangle(
        Vector2{
            drawX - 10,
            drawY - 5
        },
        Vector2{
            drawX - 5,
            drawY - 13
        },
        Vector2{
            drawX - 4,
            drawY - 4
        },
        tusk
    );


    DrawTriangle(
        Vector2{
            drawX + 10,
            drawY - 5
        },
        Vector2{
            drawX + 4,
            drawY - 4
        },
        Vector2{
            drawX + 5,
            drawY - 13
        },
        tusk
    );


    const float step =
        std::sin(
            walkPhase
        )
        *
        4.0f;


    DrawLineEx(
        Vector2{
            drawX - 11,
            drawY + 30
        },
        Vector2{
            drawX - 13 + step,
            drawY + 43
        },
        6,
        skin
    );


    DrawLineEx(
        Vector2{
            drawX + 11,
            drawY + 30
        },
        Vector2{
            drawX + 13 - step,
            drawY + 43
        },
        6,
        skin
    );
}


void Enemy::DrawBeast(
    float drawX,
    float drawY
) const
{
    const Color fur =
    {
        145,
        65,
        60,
        255
    };


    const Color dark =
    {
        65,
        30,
        30,
        255
    };


    const Color eye =
    {
        255,
        205,
        80,
        255
    };


    drawY +=
        std::sin(
            walkPhase * 1.2f
        )
        *
        2.0f;


    DrawEllipse(
        static_cast<int>(drawX),
        static_cast<int>(
            drawY + 5
        ),
        27,
        16,
        fur
    );


    DrawCircle(
        static_cast<int>(
            drawX - 22
        ),
        static_cast<int>(
            drawY - 12
        ),
        16,
        fur
    );


    DrawEllipse(
        static_cast<int>(
            drawX - 30
        ),
        static_cast<int>(
            drawY - 7
        ),
        9,
        6,
        dark
    );


    DrawTriangle(
        Vector2{
            drawX - 30,
            drawY - 24
        },
        Vector2{
            drawX - 34,
            drawY - 38
        },
        Vector2{
            drawX - 20,
            drawY - 27
        },
        dark
    );


    DrawTriangle(
        Vector2{
            drawX - 14,
            drawY - 24
        },
        Vector2{
            drawX - 8,
            drawY - 36
        },
        Vector2{
            drawX - 5,
            drawY - 21
        },
        dark
    );


    DrawCircle(
        static_cast<int>(
            drawX - 27
        ),
        static_cast<int>(
            drawY - 15
        ),
        3,
        eye
    );


    const float step =
        std::sin(
            walkPhase
        )
        *
        5.0f;


    DrawLineEx(
        Vector2{
            drawX - 16,
            drawY + 15
        },
        Vector2{
            drawX - 16 + step,
            drawY + 32
        },
        5,
        dark
    );


    DrawLineEx(
        Vector2{
            drawX + 15,
            drawY + 15
        },
        Vector2{
            drawX + 15 - step,
            drawY + 32
        },
        5,
        dark
    );


    DrawLineEx(
        Vector2{
            drawX + 22,
            drawY
        },
        Vector2{
            drawX + 35,
            drawY - 8
        },
        4,
        dark
    );


    DrawLineEx(
        Vector2{
            drawX + 35,
            drawY - 8
        },
        Vector2{
            drawX + 42,
            drawY - 2
        },
        4,
        dark
    );
}


void Enemy::Draw() const
{
    float drawX = x;

    float drawY = y;


    if (stunned)
    {
        drawX +=
            (
                (stunTimer / 4) % 2
                ==
                0
            )
            ?
            4.0f
            :
            -4.0f;
    }


    if (movementStunTimer > 0)
    {
        // Small rings make Quill Stun / Freeze visually distinct
        // from the normal between-word stagger.
        DrawCircleLines(
            static_cast<int>(drawX),
            static_cast<int>(drawY - 20),
            25.0f,
            Color{
                165,
                205,
                235,
                220
            }
        );


        DrawCircleLines(
            static_cast<int>(drawX),
            static_cast<int>(drawY - 20),
            30.0f,
            Color{
                105,
                145,
                205,
                150
            }
        );
    }


    if (defeated)
    {
        const char* text =
            "DEFEATED!";


        constexpr int fontSize =
            28;


        const int width =
            MeasureGameText(
                text,
                fontSize
            );


        DrawGameText(
            text,
            static_cast<int>(
                drawX
                -
                width / 2
            ),
            static_cast<int>(
                drawY - 50
            ),
            fontSize,
            Color{
                255,
                225,
                90,
                255
            }
        );


        return;
    }


    DrawSpriteEnemy(
        drawX,
        drawY
    );

}