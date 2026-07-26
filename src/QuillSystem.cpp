#include "QuillSystem.h"

#include "GameFont.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>


QuillSystem::QuillSystem()
{
    Reset();
}


void QuillSystem::Reset()
{
    stunUnlocked = false;
    freezeUnlocked = false;
    eraseUnlocked = false;
    rewriteUnlocked = false;

    stunMaxCharges = 0;
    freezeMaxCharges = 0;

    stunCooldowns[0] = 0;
    stunCooldowns[1] = 0;

    freezeReadyCharges = 0;
    freezeChargeProgress = 0;

    eraseCharge = 0;
    rewriteCharge = 0;

    readyPulse = 0.0f;
}


void QuillSystem::SyncUnlocks(
    int wordsRecovered
)
{
    const int oldStunMax =
        stunMaxCharges;

    const int oldFreezeMax =
        freezeMaxCharges;


    stunUnlocked =
        wordsRecovered
        >=
        STUN_UNLOCK_WORDS;

    freezeUnlocked =
        wordsRecovered
        >=
        FREEZE_UNLOCK_WORDS;

    eraseUnlocked =
        wordsRecovered
        >=
        ERASE_UNLOCK_WORDS;

    rewriteUnlocked =
        wordsRecovered
        >=
        REWRITE_UNLOCK_WORDS;


    stunMaxCharges =
        stunUnlocked
        ?
        (
            wordsRecovered
            >=
            SECOND_STUN_UNLOCK_WORDS
            ?
            2
            :
            1
        )
        :
        0;


    freezeMaxCharges =
        freezeUnlocked
        ?
        (
            wordsRecovered
            >=
            SECOND_FREEZE_UNLOCK_WORDS
            ?
            2
            :
            1
        )
        :
        0;


    if (
        stunMaxCharges
        >
        oldStunMax
    )
    {
        for (
            int index = oldStunMax;
            index < stunMaxCharges;
            index++
        )
        {
            stunCooldowns[index] = 0;
        }
    }


    if (
        oldFreezeMax == 0
        &&
        freezeMaxCharges > 0
    )
    {
        freezeReadyCharges = 0;
        freezeChargeProgress = 0;
    }


    freezeReadyCharges =
        std::clamp(
            freezeReadyCharges,
            0,
            freezeMaxCharges
        );


    freezeChargeProgress =
        std::clamp(
            freezeChargeProgress,
            0,
            FREEZE_WORD_COST - 1
        );


    eraseCharge =
        std::clamp(
            eraseCharge,
            0,
            ERASE_WORD_COST
        );


    rewriteCharge =
        std::clamp(
            rewriteCharge,
            0,
            REWRITE_WORD_COST
        );
}


void QuillSystem::Update(
    bool gameplayActive
)
{
    readyPulse += 0.08f;


    if (
        !gameplayActive
        ||
        !stunUnlocked
    )
    {
        return;
    }


    for (
        int index = 0;
        index < stunMaxCharges;
        index++
    )
    {
        if (stunCooldowns[index] > 0)
        {
            stunCooldowns[index]--;
        }
    }
}


void QuillSystem::OnWordsRecovered(
    int count
)
{
    if (count <= 0)
    {
        return;
    }


    if (freezeUnlocked)
    {
        for (
            int index = 0;
            index < count;
            index++
        )
        {
            if (
                freezeReadyCharges
                >=
                freezeMaxCharges
            )
            {
                freezeChargeProgress = 0;
                break;
            }


            freezeChargeProgress++;


            if (
                freezeChargeProgress
                >=
                FREEZE_WORD_COST
            )
            {
                freezeReadyCharges++;
                freezeChargeProgress = 0;
            }
        }
    }


    if (eraseUnlocked)
    {
        eraseCharge =
            std::min(
                ERASE_WORD_COST,
                eraseCharge + count
            );
    }


    if (rewriteUnlocked)
    {
        rewriteCharge =
            std::min(
                REWRITE_WORD_COST,
                rewriteCharge + count
            );
    }
}


bool QuillSystem::HasQuill() const
{
    return stunUnlocked;
}


bool QuillSystem::IsStunUnlocked() const
{
    return stunUnlocked;
}


bool QuillSystem::IsFreezeUnlocked() const
{
    return freezeUnlocked;
}


bool QuillSystem::IsEraseUnlocked() const
{
    return eraseUnlocked;
}


bool QuillSystem::IsRewriteUnlocked() const
{
    return rewriteUnlocked;
}


int QuillSystem::FindReadyStunSlot() const
{
    for (
        int index = 0;
        index < stunMaxCharges;
        index++
    )
    {
        if (stunCooldowns[index] <= 0)
        {
            return index;
        }
    }


    return -1;
}


bool QuillSystem::IsStunReady() const
{
    return
        stunUnlocked
        &&
        FindReadyStunSlot() >= 0;
}


bool QuillSystem::IsFreezeReady() const
{
    return
        freezeUnlocked
        &&
        freezeReadyCharges > 0;
}


bool QuillSystem::IsEraseReady() const
{
    return
        eraseUnlocked
        &&
        eraseCharge
        >=
        ERASE_WORD_COST;
}


bool QuillSystem::IsRewriteReady() const
{
    return
        rewriteUnlocked
        &&
        rewriteCharge
        >=
        REWRITE_WORD_COST;
}


bool QuillSystem::UseStun()
{
    const int slot =
        FindReadyStunSlot();


    if (slot < 0)
    {
        return false;
    }


    stunCooldowns[slot] =
        STUN_COOLDOWN_FRAMES;


    return true;
}


bool QuillSystem::UseFreeze()
{
    if (!IsFreezeReady())
    {
        return false;
    }


    freezeReadyCharges--;


    return true;
}


bool QuillSystem::UseErase()
{
    if (!IsEraseReady())
    {
        return false;
    }


    eraseCharge = 0;


    return true;
}


bool QuillSystem::UseRewrite()
{
    if (!IsRewriteReady())
    {
        return false;
    }


    rewriteCharge = 0;


    return true;
}


int QuillSystem::GetStunMaxCharges() const
{
    return stunMaxCharges;
}


int QuillSystem::GetStunReadyCharges() const
{
    int ready = 0;


    for (
        int index = 0;
        index < stunMaxCharges;
        index++
    )
    {
        if (stunCooldowns[index] <= 0)
        {
            ready++;
        }
    }


    return ready;
}


int QuillSystem::GetFreezeMaxCharges() const
{
    return freezeMaxCharges;
}


int QuillSystem::GetFreezeReadyCharges() const
{
    return freezeReadyCharges;
}


int QuillSystem::GetFreezeChargeProgress() const
{
    return freezeChargeProgress;
}


int QuillSystem::GetEraseCharge() const
{
    return eraseCharge;
}


int QuillSystem::GetRewriteCharge() const
{
    return rewriteCharge;
}


float QuillSystem::GetEraseChargeRatio() const
{
    return ClampRatio(
        static_cast<float>(
            eraseCharge
        )
        /
        static_cast<float>(
            ERASE_WORD_COST
        )
    );
}


float QuillSystem::GetRewriteChargeRatio() const
{
    return ClampRatio(
        static_cast<float>(
            rewriteCharge
        )
        /
        static_cast<float>(
            REWRITE_WORD_COST
        )
    );
}


float QuillSystem::ClampRatio(
    float value
)
{
    return std::clamp(
        value,
        0.0f,
        1.0f
    );
}


std::string QuillSystem::Normalize(
    const std::string& value
)
{
    std::string result;


    for (
        unsigned char character
        :
        value
    )
    {
        if (
            character >= 'A'
            &&
            character <= 'Z'
        )
        {
            result +=
                static_cast<char>(
                    character - 'A' + 'a'
                );
        }

        else if (
            character >= 'a'
            &&
            character <= 'z'
        )
        {
            result +=
                static_cast<char>(
                    character
                );
        }
    }


    return result;
}


bool QuillSystem::CommandReady(
    const std::string& command
) const
{
    if (command == "stun")
    {
        return IsStunReady();
    }


    if (command == "freeze")
    {
        return IsFreezeReady();
    }


    if (command == "erase")
    {
        return IsEraseReady();
    }


    if (command == "rewrite")
    {
        return IsRewriteReady();
    }


    return false;
}


std::string QuillSystem::GetMatchingCommand(
    const std::string& typed
) const
{
    const std::string normalized =
        Normalize(
            typed
        );


    if (normalized.empty())
    {
        return "";
    }


    static const char* COMMANDS[] =
    {
        "stun",
        "freeze",
        "erase",
        "rewrite"
    };


    for (
        const char* command
        :
        COMMANDS
    )
    {
        const std::string commandText =
            command;


        if (!CommandReady(commandText))
        {
            continue;
        }


        if (
            normalized.size()
            <=
            commandText.size()
            &&
            commandText.compare(
                0,
                normalized.size(),
                normalized
            )
            ==
            0
        )
        {
            return commandText;
        }
    }


    return "";
}


std::string QuillSystem::GetCompletedCommand(
    const std::string& typed
) const
{
    const std::string normalized =
        Normalize(
            typed
        );


    if (CommandReady(normalized))
    {
        return normalized;
    }


    return "";
}


void QuillSystem::DrawChargePips(
    int x,
    int y,
    int readyCharges,
    int maxCharges
) const
{
    for (
        int index = 0;
        index < maxCharges;
        index++
    )
    {
        const bool filled =
            index < readyCharges;


        DrawCircle(
            x + index * 18,
            y,
            6.0f,
            filled
            ?
            Color{
                235,
                200,
                85,
                255
            }
            :
            Color{
                65,
                65,
                75,
                255
            }
        );


        DrawCircleLines(
            x + index * 18,
            y,
            6.0f,
            Color{
                175,
                155,
                100,
                255
            }
        );
    }
}


void QuillSystem::DrawAbilityLine(
    int x,
    int y,
    const char* name,
    bool unlocked,
    bool ready,
    const std::string& typed,
    const std::string& status
) const
{
    std::string upperName =
        name;


    for (
        char& character
        :
        upperName
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
                    character - 'a' + 'A'
                );
        }
    }


    if (!unlocked)
    {
        DrawGameText(
            upperName,
            x,
            y,
            19,
            Color{
                75,
                75,
                84,
                255
            }
        );


        return;
    }


    const std::string command =
        Normalize(
            name
        );


    const std::string normalizedTyped =
        Normalize(
            typed
        );


    int highlighted = 0;


    if (
        ready
        &&
        !normalizedTyped.empty()
        &&
        normalizedTyped.size()
        <=
        command.size()
        &&
        command.compare(
            0,
            normalizedTyped.size(),
            normalizedTyped
        )
        ==
        0
    )
    {
        highlighted =
            static_cast<int>(
                normalizedTyped.size()
            );
    }


    const std::string completed =
        upperName.substr(
            0,
            highlighted
        );


    const std::string remaining =
        upperName.substr(
            highlighted
        );


    DrawGameText(
        completed,
        x,
        y,
        19,
        Color{
            250,
            210,
            80,
            255
        }
    );


    DrawGameText(
        remaining,
        x
        +
        MeasureGameText(
            completed,
            19
        ),
        y,
        19,
        ready
        ?
        Color{
            225,
            225,
            225,
            255
        }
        :
        Color{
            145,
            145,
            155,
            255
        }
    );


    if (!status.empty())
    {
        DrawGameText(
            status,
            x + 98,
            y,
            17,
            ready
            ?
            Color{
                220,
                195,
                110,
                255
            }
            :
            Color{
                130,
                130,
                140,
                255
            }
        );
    }
}


void QuillSystem::DrawGoldenQuill(
    int x,
    int y,
    int width,
    int height,
    bool ready,
    float fillRatio
) const
{
    const float ratio =
        ClampRatio(
            fillRatio
        );


    const float pulse =
        ready
        ?
        (
            0.5f
            +
            0.5f
            *
            std::sin(
                readyPulse
            )
        )
        :
        0.0f;


    const Color white =
    {
        225,
        225,
        220,
        255
    };


    const Color gold =
    {
        static_cast<unsigned char>(
            225 + pulse * 25.0f
        ),
        static_cast<unsigned char>(
            185 + pulse * 40.0f
        ),
        static_cast<unsigned char>(
            65 + pulse * 45.0f
        ),
        255
    };


    DrawRectangle(
        x,
        y,
        width,
        height,
        Color{
            18,
            18,
            28,
            215
        }
    );


    DrawRectangleLinesEx(
        Rectangle{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(width),
            static_cast<float>(height)
        },
        2.0f,
        Color{
            155,
            135,
            90,
            255
        }
    );


    const int centerX =
        x + width / 2;


    const int featherTop =
        y + 7;


    const int featherBottom =
        y + height - 18;


    auto drawFeather =
        [&](
            Color color
        )
        {
            // Shaft.
            DrawLineEx(
                Vector2{
                    static_cast<float>(
                        centerX - 7
                    ),
                    static_cast<float>(
                        featherBottom + 8
                    )
                },
                Vector2{
                    static_cast<float>(
                        centerX + 6
                    ),
                    static_cast<float>(
                        featherTop + 8
                    )
                },
                4.0f,
                color
            );


            // Pointed feather crown.
            DrawTriangle(
                Vector2{
                    static_cast<float>(
                        centerX + 6
                    ),
                    static_cast<float>(
                        featherTop + 5
                    )
                },
                Vector2{
                    static_cast<float>(
                        centerX + 18
                    ),
                    static_cast<float>(
                        featherTop + 18
                    )
                },
                Vector2{
                    static_cast<float>(
                        centerX + 3
                    ),
                    static_cast<float>(
                        featherTop + 22
                    )
                },
                color
            );


            // Feather barbs on both sides.
            for (
                int index = 0;
                index < 5;
                index++
            )
            {
                const float t =
                    static_cast<float>(
                        index
                    )
                    /
                    4.0f;


                const float shaftX =
                    static_cast<float>(
                        centerX + 4
                    )
                    -
                    t * 8.0f;


                const float shaftY =
                    static_cast<float>(
                        featherTop + 18
                    )
                    +
                    t
                    *
                    static_cast<float>(
                        featherBottom
                        -
                        featherTop
                        -
                        18
                    );


                const float spread =
                    18.0f
                    -
                    t * 6.0f;


                DrawTriangle(
                    Vector2{
                        shaftX,
                        shaftY
                    },
                    Vector2{
                        shaftX - spread,
                        shaftY - 7.0f
                    },
                    Vector2{
                        shaftX - 2.0f,
                        shaftY + 8.0f
                    },
                    color
                );


                DrawTriangle(
                    Vector2{
                        shaftX,
                        shaftY - 3.0f
                    },
                    Vector2{
                        shaftX + spread,
                        shaftY - 10.0f
                    },
                    Vector2{
                        shaftX + 2.0f,
                        shaftY + 7.0f
                    },
                    color
                );
            }


            // Writing nib.
            DrawTriangle(
                Vector2{
                    static_cast<float>(
                        centerX - 11
                    ),
                    static_cast<float>(
                        featherBottom + 10
                    )
                },
                Vector2{
                    static_cast<float>(
                        centerX - 3
                    ),
                    static_cast<float>(
                        featherBottom
                    )
                },
                Vector2{
                    static_cast<float>(
                        centerX - 9
                    ),
                    static_cast<float>(
                        featherBottom + 17
                    )
                },
                color
            );
        };


    drawFeather(
        white
    );


    const int innerHeight =
        height - 8;


    const int fillHeight =
        static_cast<int>(
            innerHeight * ratio
        );


    if (fillHeight > 0)
    {
        BeginScissorMode(
            x + 2,
            y + height - 4 - fillHeight,
            width - 4,
            fillHeight
        );


        drawFeather(
            gold
        );


        EndScissorMode();
    }


    if (ready)
    {
        DrawCircleLines(
            centerX,
            y + height / 2,
            static_cast<float>(
                width / 2 - 5
            ),
            Color{
                240,
                205,
                90,
                static_cast<unsigned char>(
                    110 + pulse * 100.0f
                )
            }
        );
    }
}


void QuillSystem::DrawHUD(
    int screenWidth,
    int screenHeight,
    bool showQuill,
    const std::string& typed
) const
{
    if (
        !showQuill
        ||
        !HasQuill()
    )
    {
        return;
    }


    const int panelWidth = 270;


    const int panelHeight =
        rewriteUnlocked
        ?
        152
        :
        112;


    const int x =
        screenWidth
        -
        panelWidth
        -
        14;


    const int y =
        screenHeight
        -
        panelHeight
        -
        14;


    DrawRectangle(
        x,
        y,
        panelWidth,
        panelHeight,
        Color{
            12,
            12,
            20,
            205
        }
    );


    DrawRectangleLinesEx(
        Rectangle{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(panelWidth),
            static_cast<float>(panelHeight)
        },
        2.0f,
        Color{
            95,
            90,
            105,
            255
        }
    );


    DrawAbilityLine(
        x + 12,
        y + 10,
        "STUN",
        stunUnlocked,
        IsStunReady(),
        typed,
        ""
    );


    if (stunUnlocked)
    {
        DrawChargePips(
            x + 205,
            y + 20,
            GetStunReadyCharges(),
            GetStunMaxCharges()
        );
    }


    std::string freezeStatus;


    if (freezeUnlocked)
    {
        freezeStatus =
            std::to_string(
                freezeReadyCharges
            )
            +
            "/"
            +
            std::to_string(
                freezeMaxCharges
            );


        if (
            freezeReadyCharges
            <
            freezeMaxCharges
        )
        {
            freezeStatus +=
                " "
                +
                std::to_string(
                    freezeChargeProgress
                )
                +
                "/"
                +
                std::to_string(
                    FREEZE_WORD_COST
                );
        }
    }


    DrawAbilityLine(
        x + 12,
        y + 36,
        "FREEZE",
        freezeUnlocked,
        IsFreezeReady(),
        typed,
        freezeStatus
    );


    DrawAbilityLine(
        x + 12,
        y + 62,
        "ERASE",
        eraseUnlocked,
        IsEraseReady(),
        typed,
        eraseUnlocked
        ?
        std::to_string(
            eraseCharge
        )
        +
        "/"
        +
        std::to_string(
            ERASE_WORD_COST
        )
        :
        ""
    );


    if (rewriteUnlocked)
    {
        DrawAbilityLine(
            x + 12,
            y + 88,
            "REWRITE",
            true,
            IsRewriteReady(),
            typed,
            std::to_string(
                rewriteCharge
            )
            +
            "/"
            +
            std::to_string(
                REWRITE_WORD_COST
            )
        );


        DrawGoldenQuill(
            x + panelWidth - 62,
            y + 84,
            48,
            58,
            IsRewriteReady(),
            GetRewriteChargeRatio()
        );
    }
}