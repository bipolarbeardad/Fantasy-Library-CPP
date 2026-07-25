#pragma once

#include "raylib.h"

class QuillSystem
{
public:
    QuillSystem();

    static constexpr int STUN_UNLOCK_WORDS = 50;
    static constexpr int FREEZE_UNLOCK_WORDS = 100;
    static constexpr int ERASE_UNLOCK_WORDS = 250;
    static constexpr int REWRITE_UNLOCK_WORDS = 500;

    static constexpr int STUN_DURATION_FRAMES = 180;
    static constexpr int STUN_COOLDOWN_FRAMES = 900;

    static constexpr int FREEZE_DURATION_FRAMES = 240;
    static constexpr int FREEZE_WORD_COST = 15;

    // Easy-to-tune placeholders until playtesting.
    static constexpr int ERASE_WORD_COST = 10;
    static constexpr int REWRITE_WORD_COST = 30;

    void Reset();
    void SyncUnlocks(int wordsRecovered);

    // Pass true only during unpaused Adventure gameplay.
    void Update(bool gameplayActive);

    // Adds charge to word-powered abilities.
    void OnWordsRecovered(int count);

    bool HasQuill() const;

    bool IsStunUnlocked() const;
    bool IsFreezeUnlocked() const;
    bool IsEraseUnlocked() const;
    bool IsRewriteUnlocked() const;

    bool IsStunReady() const;
    bool IsFreezeReady() const;
    bool IsEraseReady() const;
    bool IsRewriteReady() const;

    bool UseStun();
    bool UseFreeze();
    bool UseErase();
    bool UseRewrite();

    int GetStunCooldownFrames() const;
    float GetStunCooldownRatio() const;

    int GetFreezeCharge() const;
    int GetEraseCharge() const;
    int GetRewriteCharge() const;

    float GetFreezeChargeRatio() const;
    float GetEraseChargeRatio() const;
    float GetRewriteChargeRatio() const;

    void DrawHUD(
        int screenWidth,
        int screenHeight,
        bool showQuill
    ) const;

private:
    bool stunUnlocked;
    bool freezeUnlocked;
    bool eraseUnlocked;
    bool rewriteUnlocked;

    int stunCooldownRemaining;
    int freezeCharge;
    int eraseCharge;
    int rewriteCharge;

    float readyPulse;

    static float ClampRatio(float value);

    void DrawAbilityBox(
        int x,
        int y,
        int width,
        int height,
        const char* keyLabel,
        const char* name,
        bool unlocked,
        bool ready,
        float fillRatio
    ) const;

    void DrawGoldenQuill(
        int x,
        int y,
        int width,
        int height,
        bool ready,
        float fillRatio
    ) const;
};