#include "QuillSystem.h"

#include "GameFont.h"

#include <algorithm>
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

    stunCooldownRemaining = 0;

    freezeCharge = 0;
    eraseCharge = 0;
    rewriteCharge = 0;

    readyPulse = 0.0f;
}


void QuillSystem::SyncUnlocks(
    int wordsRecovered
)
{
    stunUnlocked =
        wordsRecovered >= STUN_UNLOCK_WORDS;

    freezeUnlocked =
        wordsRecovered >= FREEZE_UNLOCK_WORDS;

    eraseUnlocked =
        wordsRecovered >= ERASE_UNLOCK_WORDS;

    rewriteUnlocked =
        wordsRecovered >= REWRITE_UNLOCK_WORDS;

    freezeCharge =
        std::clamp(
            freezeCharge,
            0,
            FREEZE_WORD_COST
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
        gameplayActive
        &&
        stunUnlocked
        &&
        stunCooldownRemaining > 0
    )
    {
        stunCooldownRemaining--;
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
        freezeCharge =
            std::min(
                FREEZE_WORD_COST,
                freezeCharge + count
            );
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


bool QuillSystem::IsStunReady() const
{
    return
        stunUnlocked
        &&
        stunCooldownRemaining <= 0;
}


bool QuillSystem::IsFreezeReady() const
{
    return
        freezeUnlocked
        &&
        freezeCharge >= FREEZE_WORD_COST;
}


bool QuillSystem::IsEraseReady() const
{
    return
        eraseUnlocked
        &&
        eraseCharge >= ERASE_WORD_COST;
}


bool QuillSystem::IsRewriteReady() const
{
    return
        rewriteUnlocked
        &&
        rewriteCharge >= REWRITE_WORD_COST;
}


bool QuillSystem::UseStun()
{
    if (!IsStunReady())
    {
        return false;
    }

    stunCooldownRemaining =
        STUN_COOLDOWN_FRAMES;

    return true;
}


bool QuillSystem::UseFreeze()
{
    if (!IsFreezeReady())
    {
        return false;
    }

    freezeCharge = 0;

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


int QuillSystem::GetStunCooldownFrames() const
{
    return stunCooldownRemaining;
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


float QuillSystem::GetStunCooldownRatio() const
{
    if (!stunUnlocked)
    {
        return 0.0f;
    }

    if (stunCooldownRemaining <= 0)
    {
        return 1.0f;
    }

    return ClampRatio(
        1.0f
        -
        static_cast<float>(
            stunCooldownRemaining
        )
        /
        static_cast<float>(
            STUN_COOLDOWN_FRAMES
        )
    );
}


int QuillSystem::GetFreezeCharge() const
{
    return freezeCharge;
}


int QuillSystem::GetEraseCharge() const
{
    return eraseCharge;
}


int QuillSystem::GetRewriteCharge() const
{
    return rewriteCharge;
}


float QuillSystem::GetFreezeChargeRatio() const
{
    return ClampRatio(
        static_cast<float>(freezeCharge)
        /
        static_cast<float>(FREEZE_WORD_COST)
    );
}


float QuillSystem::GetEraseChargeRatio() const
{
    return ClampRatio(
        static_cast<float>(eraseCharge)
        /
        static_cast<float>(ERASE_WORD_COST)
    );
}


float QuillSystem::GetRewriteChargeRatio() const
{
    return ClampRatio(
        static_cast<float>(rewriteCharge)
        /
        static_cast<float>(REWRITE_WORD_COST)
    );
}


void QuillSystem::DrawAbilityBox(
    int x,
    int y,
    int width,
    int height,
    const char* keyLabel,
    const char* name,
    bool unlocked,
    bool ready,
    float fillRatio
) const
{
    DrawRectangle(
        x,
        y,
        width,
        height,
        Color{18, 18, 28, 220}
    );

    DrawRectangleLinesEx(
        Rectangle{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(width),
            static_cast<float>(height)
        },
        2.0f,
        unlocked
        ?
        Color{150, 135, 95, 255}
        :
        Color{60, 60, 68, 255}
    );

    if (unlocked)
    {
        const int innerWidth =
            width - 8;

        DrawRectangle(
            x + 4,
            y + height - 10,
            static_cast<int>(
                innerWidth
                *
                ClampRatio(fillRatio)
            ),
            6,
            ready
            ?
            Color{230, 200, 95, 255}
            :
            Color{115, 105, 90, 255}
        );
    }

    DrawGameText(
        unlocked ? keyLabel : "-",
        x + 7,
        y + 6,
        18,
        unlocked
        ?
        Color{240, 220, 150, 255}
        :
        Color{85, 85, 92, 255}
    );

    DrawGameText(
        unlocked ? name : "LOCKED",
        x + 31,
        y + 6,
        18,
        unlocked
        ?
        (
            ready
            ?
            Color{255, 235, 155, 255}
            :
            Color{190, 190, 195, 255}
        )
        :
        Color{85, 85, 92, 255}
    );
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
        ClampRatio(fillRatio);

    const float pulse =
        ready
        ?
        (
            0.5f
            +
            0.5f
            *
            std::sin(readyPulse)
        )
        :
        0.0f;

    const Color whiteQuill =
    {
        220,
        220,
        215,
        255
    };

    const Color goldQuill =
    {
        static_cast<unsigned char>(
            ready
            ?
            235 + pulse * 20.0f
            :
            235
        ),
        static_cast<unsigned char>(
            ready
            ?
            195 + pulse * 30.0f
            :
            195
        ),
        static_cast<unsigned char>(
            ready
            ?
            75 + pulse * 45.0f
            :
            75
        ),
        255
    };

    DrawRectangle(
        x,
        y,
        width,
        height,
        Color{18, 18, 28, 220}
    );

    DrawRectangleLinesEx(
        Rectangle{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(width),
            static_cast<float>(height)
        },
        2.0f,
        Color{165, 145, 85, 255}
    );

    const int centerX =
        x + width / 2;

    const int topY =
        y + 7;

    const int bottomY =
        y + height - 18;

    auto drawQuill =
        [&](
            Color color
        )
        {
            DrawLineEx(
                Vector2{
                    static_cast<float>(centerX - 8),
                    static_cast<float>(bottomY)
                },
                Vector2{
                    static_cast<float>(centerX + 10),
                    static_cast<float>(topY + 6)
                },
                4.0f,
                color
            );

            DrawTriangle(
                Vector2{
                    static_cast<float>(centerX + 9),
                    static_cast<float>(topY + 7)
                },
                Vector2{
                    static_cast<float>(centerX + 20),
                    static_cast<float>(topY)
                },
                Vector2{
                    static_cast<float>(centerX + 13),
                    static_cast<float>(topY + 23)
                },
                color
            );

            DrawTriangle(
                Vector2{
                    static_cast<float>(centerX + 8),
                    static_cast<float>(topY + 8)
                },
                Vector2{
                    static_cast<float>(centerX - 1),
                    static_cast<float>(topY + 4)
                },
                Vector2{
                    static_cast<float>(centerX + 10),
                    static_cast<float>(topY + 23)
                },
                color
            );
        };

    drawQuill(
        whiteQuill
    );

    const int fillHeight =
        static_cast<int>(
            (height - 12)
            *
            ratio
        );

    if (fillHeight > 0)
    {
        BeginScissorMode(
            x + 2,
            y + height - 6 - fillHeight,
            width - 4,
            fillHeight
        );

        drawQuill(
            goldQuill
        );

        EndScissorMode();
    }

    DrawGameText(
        "4",
        x + 6,
        y + 6,
        18,
        Color{240, 220, 150, 255}
    );

    DrawGameText(
        "REWRITE",
        x + 10,
        y + height - 20,
        15,
        ready
        ?
        goldQuill
        :
        Color{190, 190, 195, 255}
    );
}


void QuillSystem::DrawHUD(
    int screenWidth,
    int screenHeight,
    bool showQuill
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

    constexpr int boxWidth = 112;
    constexpr int boxHeight = 36;
    constexpr int gap = 8;
    constexpr int rewriteWidth = 76;

    const int totalWidth =
        boxWidth * 3
        +
        gap * 3
        +
        rewriteWidth;

    const int startX =
        std::max(
            12,
            screenWidth - totalWidth - 12
        );

    const int y =
        std::max(
            12,
            screenHeight - boxHeight - 14
        );

    DrawAbilityBox(
        startX,
        y,
        boxWidth,
        boxHeight,
        "1",
        "STUN",
        stunUnlocked,
        IsStunReady(),
        GetStunCooldownRatio()
    );

    DrawAbilityBox(
        startX + boxWidth + gap,
        y,
        boxWidth,
        boxHeight,
        "2",
        "FREEZE",
        freezeUnlocked,
        IsFreezeReady(),
        GetFreezeChargeRatio()
    );

    DrawAbilityBox(
        startX + (boxWidth + gap) * 2,
        y,
        boxWidth,
        boxHeight,
        "3",
        "ERASE",
        eraseUnlocked,
        IsEraseReady(),
        GetEraseChargeRatio()
    );

    if (rewriteUnlocked)
    {
        DrawGoldenQuill(
            startX + (boxWidth + gap) * 3,
            y - 24,
            rewriteWidth,
            60,
            IsRewriteReady(),
            GetRewriteChargeRatio()
        );
    }
}