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
            const Vector2 nibTip =
            {
                static_cast<float>(
                    centerX - 10
                ),
                static_cast<float>(
                    featherBottom + 12
                )
            };


            const float length =
                static_cast<float>(
                    height - 15
                );


            const float angle =
                -58.0f
                *
                DEG2RAD;


            const Vector2 direction =
            {
                std::cos(
                    angle
                ),
                std::sin(
                    angle
                )
            };


            const Vector2 side =
            {
                -direction.y,
                direction.x
            };


            const Vector2 featherTip =
            {
                nibTip.x
                +
                direction.x
                *
                length,

                nibTip.y
                +
                direction.y
                *
                length
            };


            DrawLineEx(
                nibTip,
                featherTip,
                3.2f,
                color
            );


            constexpr int sectionCount =
                6;


            for (
                int index = 0;
                index < sectionCount;
                index++
            )
            {
                const float t0 =
                    0.28f
                    +
                    static_cast<float>(
                        index
                    )
                    /
                    static_cast<float>(
                        sectionCount
                    )
                    *
                    0.67f;


                const float t1 =
                    0.28f
                    +
                    static_cast<float>(
                        index + 1
                    )
                    /
                    static_cast<float>(
                        sectionCount
                    )
                    *
                    0.67f;


                const Vector2 center0 =
                {
                    nibTip.x
                    +
                    direction.x
                    *
                    length
                    *
                    t0,

                    nibTip.y
                    +
                    direction.y
                    *
                    length
                    *
                    t0
                };


                const Vector2 center1 =
                {
                    nibTip.x
                    +
                    direction.x
                    *
                    length
                    *
                    t1,

                    nibTip.y
                    +
                    direction.y
                    *
                    length
                    *
                    t1
                };


                const float taper0 =
                    std::sin(
                        t0
                        *
                        PI
                    );


                const float taper1 =
                    std::sin(
                        t1
                        *
                        PI
                    );


                const float width0 =
                    9.0f
                    *
                    taper0;


                const float width1 =
                    9.0f
                    *
                    taper1;


                const Vector2 left0 =
                {
                    center0.x
                    +
                    side.x
                    *
                    width0,

                    center0.y
                    +
                    side.y
                    *
                    width0
                };


                const Vector2 left1 =
                {
                    center1.x
                    +
                    side.x
                    *
                    width1,

                    center1.y
                    +
                    side.y
                    *
                    width1
                };


                const Vector2 right0 =
                {
                    center0.x
                    -
                    side.x
                    *
                    width0
                    *
                    0.84f,

                    center0.y
                    -
                    side.y
                    *
                    width0
                    *
                    0.84f
                };


                const Vector2 right1 =
                {
                    center1.x
                    -
                    side.x
                    *
                    width1
                    *
                    0.84f,

                    center1.y
                    -
                    side.y
                    *
                    width1
                    *
                    0.84f
                };


                DrawTriangle(
                    center0,
                    left0,
                    left1,
                    color
                );


                DrawTriangle(
                    center0,
                    left1,
                    center1,
                    color
                );


                DrawTriangle(
                    center0,
                    center1,
                    right1,
                    color
                );


                DrawTriangle(
                    center0,
                    right1,
                    right0,
                    color
                );
            }


            // Narrow writing nib.
            DrawTriangle(
                Vector2{
                    nibTip.x - 3.0f,
                    nibTip.y + 2.0f
                },
                Vector2{
                    nibTip.x + 3.0f,
                    nibTip.y - 5.0f
                },
                Vector2{
                    nibTip.x - 1.0f,
                    nibTip.y + 9.0f
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
    int playerX,
    int playerY,
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


    const int panelWidth =
        205;


    const int panelHeight =
        rewriteUnlocked
        ?
        126
        :
        96;


    // Anchor the HUD to the Word Seeker's side of the page.
    // Because the player sits close to the left edge, centering a
    // 205px panel under playerX would push it off-screen. Instead,
    // start just inside the left edge and keep it near the player.
    int x =
        std::max(
            12,
            playerX - 70
        );


    int y =
        playerY
        +
        62;


    // Keep the full panel on-screen at all supported resolutions.
    x =
        std::clamp(
            x,
            12,
            std::max(
                12,
                screenWidth
                -
                panelWidth
                -
                12
            )
        );


    y =
        std::clamp(
            y,
            12,
            std::max(
                12,
                screenHeight
                -
                panelHeight
                -
                12
            )
        );


    DrawRectangle(
        x,
        y,
        panelWidth,
        panelHeight,
        Color{
            12,
            12,
            20,
            190
        }
    );


    DrawRectangleLinesEx(
        Rectangle{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(panelWidth),
            static_cast<float>(panelHeight)
        },
        1.5f,
        Color{
            95,
            90,
            105,
            225
        }
    );


    DrawAbilityLine(
        x + 10,
        y + 7,
        "STUN",
        stunUnlocked,
        IsStunReady(),
        typed,
        ""
    );


    if (stunUnlocked)
    {
        DrawChargePips(
            x + 158,
            y + 17,
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
        x + 10,
        y + 29,
        "FREEZE",
        freezeUnlocked,
        IsFreezeReady(),
        typed,
        freezeStatus
    );


    DrawAbilityLine(
        x + 10,
        y + 51,
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
            x + 10,
            y + 73,
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
            x + 158,
            y + 72,
            38,
            46,
            IsRewriteReady(),
            GetRewriteChargeRatio()
        );
    }
}