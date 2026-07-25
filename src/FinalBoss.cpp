#include "FinalBoss.h"

#include "GameFont.h"
#include "raylib.h"

#include <algorithm>
#include <cmath>


const char* FinalBoss::PHASE_WORDS[
    FinalBoss::PHASE_COUNT
] =
{
    "Forgotten",
    "Abandoned",
    "Unfinished",
    "TheEnd"
};


const char* FinalBoss::PHASE_LABELS[
    FinalBoss::PHASE_COUNT
] =
{
    "The stolen Quill scratches at the Ribbon.",
    "The Author tries to rewrite what cannot be written.",
    "The stain fractures around a truth it cannot understand.",
    "The final word waits where it always belonged."
};


const char* FinalBoss::REACTIONS[
    FinalBoss::PHASE_COUNT - 1
] =
{
    "Why won't you disappear?!",
    "I have the Quill! I can rewrite anything!",
    "What ARE you?!"
};


const char* FinalBoss::THEFT_LINES[
    FinalBoss::THEFT_LINE_COUNT
] =
{
    "All this time...",
    "You could rewrite it.",
    "Then I don't have to be The End.",
    "I only need to erase you.",
    "The Stained Author tears the Quill from the Ribbon."
};


FinalBoss::FinalBoss()
    :
    phase(0),
    finished(false),
    hitTimer(0),
    reactionTimer(0),
    wordTimer(0),
    wordTimerMax(0),
    slashTimer(0),
    pulse(0.0f)
{
    Reset();
}


int FinalBoss::GetTimerForPhase() const
{
    // Seconds at 60 FPS:
    // 10.0, 8.5, 7.0, 6.0
    switch (phase)
    {
        case 0:
            return 600;

        case 1:
            return 510;

        case 2:
            return 420;

        default:
            return 360;
    }
}


void FinalBoss::Reset()
{
    phase = 0;

    finished = false;

    hitTimer = 0;

    reactionTimer = 0;

    slashTimer = 0;

    pulse = 0.0f;

    reactionText.clear();


    ResetWordTimer();
}


void FinalBoss::ResetWordTimer()
{
    wordTimerMax =
        GetTimerForPhase();


    wordTimer =
        wordTimerMax;
}


const std::string&
FinalBoss::GetCurrentWord() const
{
    static std::string current;


    if (finished)
    {
        current.clear();

        return current;
    }


    current =
        PHASE_WORDS[
            std::clamp(
                phase,
                0,
                PHASE_COUNT - 1
            )
        ];


    return current;
}


const std::string&
FinalBoss::GetReactionText() const
{
    return reactionText;
}


const char* FinalBoss::GetTheftLine(
    int index
) const
{
    if (
        index < 0
        ||
        index >= THEFT_LINE_COUNT
    )
    {
        return "";
    }


    return THEFT_LINES[index];
}


int FinalBoss::GetTheftLineCount() const
{
    return THEFT_LINE_COUNT;
}


int FinalBoss::GetPhase() const
{
    return phase;
}


int FinalBoss::GetPhaseCount() const
{
    return PHASE_COUNT;
}


bool FinalBoss::CompleteCurrentWord()
{
    if (
        finished
        ||
        reactionTimer > 0
        ||
        slashTimer > 0
    )
    {
        return finished;
    }


    hitTimer = 70;


    if (phase < PHASE_COUNT - 1)
    {
        reactionText =
            REACTIONS[phase];


        reactionTimer =
            105;
    }


    phase++;


    if (phase >= PHASE_COUNT)
    {
        phase =
            PHASE_COUNT;

        finished =
            true;

        reactionText.clear();

        reactionTimer =
            0;

        return true;
    }


    ResetWordTimer();


    return false;
}


bool FinalBoss::IsFinished() const
{
    return finished;
}


bool FinalBoss::IsReacting() const
{
    return reactionTimer > 0;
}


bool FinalBoss::IsChargingAttack() const
{
    if (
        wordTimerMax <= 0
        ||
        finished
        ||
        IsReacting()
        ||
        slashTimer > 0
    )
    {
        return false;
    }


    return
        wordTimer
        <=
        wordTimerMax / 3;
}


bool FinalBoss::IsSlashing() const
{
    return slashTimer > 0;
}


float FinalBoss::GetTimerRatio() const
{
    if (wordTimerMax <= 0)
    {
        return 0.0f;
    }


    return std::clamp(
        static_cast<float>(
            wordTimer
        )
        /
        static_cast<float>(
            wordTimerMax
        ),
        0.0f,
        1.0f
    );
}


bool FinalBoss::Update()
{
    pulse +=
        0.025f;


    if (hitTimer > 0)
    {
        hitTimer--;
    }


    if (reactionTimer > 0)
    {
        reactionTimer--;


        if (reactionTimer == 0)
        {
            reactionText.clear();


            ResetWordTimer();
        }


        return false;
    }


    if (slashTimer > 0)
    {
        slashTimer--;


        // Apply damage once during the middle of the slash.
        if (slashTimer == 18)
        {
            return true;
        }


        if (slashTimer == 0)
        {
            ResetWordTimer();
        }


        return false;
    }


    if (finished)
    {
        return false;
    }


    if (wordTimer > 0)
    {
        wordTimer--;


        if (wordTimer == 0)
        {
            slashTimer =
                36;
        }
    }


    return false;
}


void FinalBoss::Draw() const
{
    const int centerX =
        GetScreenWidth() / 2;

    const int centerY =
        GetScreenHeight() / 2 + 25;


    const float breathing =
        std::sin(
            pulse
        )
        *
        5.0f;


    const bool struck =
        hitTimer > 0;


    const int shake =
        struck
        ?
        (
            (hitTimer / 4) % 2 == 0
            ?
            -5
            :
            5
        )
        :
        0;


    const int bossX =
        centerX + shake;

    const int bossY =
        centerY
        +
        static_cast<int>(
            breathing
        );


    DrawEllipse(
        bossX,
        bossY + 93,
        92,
        18,
        Color{
            4,
            4,
            8,
            230
        }
    );


    for (
        int index = 0;
        index < 7;
        index++
    )
    {
        const float offset =
            static_cast<float>(
                index - 3
            )
            *
            22.0f;


        DrawLineEx(
            Vector2{
                static_cast<float>(
                    bossX
                ),
                static_cast<float>(
                    bossY + 55
                )
            },
            Vector2{
                static_cast<float>(
                    bossX
                )
                +
                offset,
                static_cast<float>(
                    bossY
                    +
                    105
                    +
                    (index % 2) * 12
                )
            },
            8.0f,
            Color{
                18,
                14,
                25,
                255
            }
        );
    }


    DrawTriangle(
        Vector2{
            static_cast<float>(
                bossX - 52
            ),
            static_cast<float>(
                bossY - 8
            )
        },
        Vector2{
            static_cast<float>(
                bossX
            ),
            static_cast<float>(
                bossY + 92
            )
        },
        Vector2{
            static_cast<float>(
                bossX + 52
            ),
            static_cast<float>(
                bossY - 8
            )
        },
        Color{
            24,
            18,
            32,
            255
        }
    );


    DrawTriangle(
        Vector2{
            static_cast<float>(
                bossX - 48
            ),
            static_cast<float>(
                bossY + 8
            )
        },
        Vector2{
            static_cast<float>(
                bossX - 70
            ),
            static_cast<float>(
                bossY + 82
            )
        },
        Vector2{
            static_cast<float>(
                bossX
            ),
            static_cast<float>(
                bossY + 92
            )
        },
        Color{
            35,
            24,
            45,
            255
        }
    );


    DrawTriangle(
        Vector2{
            static_cast<float>(
                bossX + 48
            ),
            static_cast<float>(
                bossY + 8
            )
        },
        Vector2{
            static_cast<float>(
                bossX
            ),
            static_cast<float>(
                bossY + 92
            )
        },
        Vector2{
            static_cast<float>(
                bossX + 70
            ),
            static_cast<float>(
                bossY + 82
            )
        },
        Color{
            35,
            24,
            45,
            255
        }
    );


    DrawCircle(
        bossX,
        bossY - 39,
        33,
        Color{
            20,
            16,
            27,
            255
        }
    );


    DrawCircle(
        bossX - 10,
        bossY - 43,
        4,
        Color{
            210,
            175,
            90,
            255
        }
    );


    DrawCircle(
        bossX + 10,
        bossY - 43,
        4,
        Color{
            210,
            175,
            90,
            255
        }
    );


    // Arm holding the quill.
    DrawLineEx(
        Vector2{
            static_cast<float>(
                bossX + 25
            ),
            static_cast<float>(
                bossY + 4
            )
        },
        Vector2{
            static_cast<float>(
                bossX + 53
            ),
            static_cast<float>(
                bossY + 28
            )
        },
        10.0f,
        Color{
            30,
            22,
            38,
            255
        }
    );


    DrawCircle(
        bossX + 55,
        bossY + 30,
        7,
        Color{
            40,
            30,
            48,
            255
        }
    );


    // Raise the quill as the attack approaches.
    float quillLift =
        0.0f;


    if (IsChargingAttack())
    {
        quillLift =
            30.0f;
    }


    if (IsSlashing())
    {
        // Swing down sharply during the attack.
        quillLift =
            -25.0f
            +
            static_cast<float>(
                slashTimer
            );
    }


    const float shaftEndX =
        static_cast<float>(
            bossX + 101
        );


    const float shaftEndY =
        static_cast<float>(
            bossY - 63
        )
        -
        quillLift;


    DrawLineEx(
        Vector2{
            static_cast<float>(
                bossX + 54
            ),
            static_cast<float>(
                bossY + 30
            )
        },
        Vector2{
            shaftEndX,
            shaftEndY
        },
        5.0f,
        IsChargingAttack()
        ?
        Color{
            255,
            210,
            95,
            255
        }
        :
        Color{
            215,
            198,
            150,
            255
        }
    );


    DrawTriangle(
        Vector2{
            shaftEndX - 2,
            shaftEndY - 2
        },
        Vector2{
            shaftEndX + 24,
            shaftEndY - 40
        },
        Vector2{
            shaftEndX + 8,
            shaftEndY + 10
        },
        Color{
            135,
            115,
            145,
            255
        }
    );


    // Slash streak traveling toward the Word Seeker.
    if (IsSlashing())
    {
        const float progress =
            1.0f
            -
            static_cast<float>(
                slashTimer
            )
            /
            36.0f;


        const float slashX =
            static_cast<float>(
                bossX
            )
            -
            progress
            *
            static_cast<float>(
                bossX - 90
            );


        const float slashY =
            static_cast<float>(
                bossY
            )
            +
            progress
            *
            5.0f;


        DrawLineEx(
            Vector2{
                slashX + 30.0f,
                slashY - 35.0f
            },
            Vector2{
                slashX - 30.0f,
                slashY + 35.0f
            },
            9.0f,
            Color{
                125,
                90,
                150,
                235
            }
        );
    }


    DrawGameText(
        "THE STAINED AUTHOR",
        centerX
        -
        MeasureGameText(
            "THE STAINED AUTHOR",
            38
        )
        /
        2,
        38,
        38,
        Color{
            230,
            210,
            160,
            255
        }
    );


    DrawGameText(
        "WIELDING THE STOLEN QUILL",
        centerX
        -
        MeasureGameText(
            "WIELDING THE STOLEN QUILL",
            17
        )
        /
        2,
        78,
        17,
        Color{
            155,
            135,
            105,
            255
        }
    );


    if (finished)
    {
        return;
    }


    if (IsReacting())
    {
        const std::string quote =
            "\""
            +
            reactionText
            +
            "\"";


        DrawGameText(
            quote,
            centerX
            -
            MeasureGameText(
                quote,
                28
            )
            /
            2,
            105,
            28,
            Color{
                255,
                195,
                125,
                255
            }
        );


        return;
    }


    const std::string word =
        GetCurrentWord();


    std::string upper =
        word;


    for (
        char& character
        :
        upper
    )
    {
        if (
            character >= 'a'
            &&
            character <= 'z'
        )
        {
            character =
                static_cast<char>(
                    character
                    -
                    'a'
                    +
                    'A'
                );
        }
    }


    DrawGameText(
        upper,
        centerX
        -
        MeasureGameText(
            upper,
            36
        )
        /
        2,
        105,
        36,
        struck
        ?
        Color{
            255,
            220,
            90,
            255
        }
        :
        Color{
            240,
            240,
            235,
            255
        }
    );


    const int safePhase =
        std::clamp(
            phase,
            0,
            PHASE_COUNT - 1
        );


    DrawGameText(
        PHASE_LABELS[
            safePhase
        ],
        centerX
        -
        MeasureGameText(
            PHASE_LABELS[
                safePhase
            ],
            19
        )
        /
        2,
        150,
        19,
        Color{
            155,
            145,
            160,
            255
        }
    );


    // Small unobtrusive attack timer bar.
    const int barWidth =
        260;

    const int barHeight =
        8;

    const int barX =
        centerX
        -
        barWidth / 2;

    const int barY =
        185;


    DrawRectangle(
        barX,
        barY,
        barWidth,
        barHeight,
        Color{
            45,
            40,
            50,
            255
        }
    );


    DrawRectangle(
        barX,
        barY,
        static_cast<int>(
            barWidth
            *
            GetTimerRatio()
        ),
        barHeight,
        IsChargingAttack()
        ?
        Color{
            220,
            150,
            75,
            255
        }
        :
        Color{
            150,
            135,
            95,
            255
        }
    );
}