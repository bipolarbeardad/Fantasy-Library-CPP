#pragma once

#include "raylib.h"

#include <string>

class QuillSystem
{
public:
    QuillSystem();

    static constexpr int STUN_UNLOCK_WORDS = 50;
    static constexpr int FREEZE_UNLOCK_WORDS = 100;
    static constexpr int ERASE_UNLOCK_WORDS = 250;
    static constexpr int REWRITE_UNLOCK_WORDS = 500;

    static constexpr int SECOND_STUN_UNLOCK_WORDS = 250;
    static constexpr int SECOND_FREEZE_UNLOCK_WORDS = 500;

    static constexpr int STUN_DURATION_FRAMES = 180;
    static constexpr int STUN_COOLDOWN_FRAMES = 900;

    static constexpr int FREEZE_DURATION_FRAMES = 240;
    static constexpr int FREEZE_WORD_COST = 15;

    static constexpr int ERASE_WORD_COST = 10;
    static constexpr int REWRITE_WORD_COST = 30;

    void Reset();
    void SyncUnlocks(int wordsRecovered);
    void Update(bool gameplayActive);
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

    int GetStunMaxCharges() const;
    int GetStunReadyCharges() const;

    int GetFreezeMaxCharges() const;
    int GetFreezeReadyCharges() const;
    int GetFreezeChargeProgress() const;

    int GetEraseCharge() const;
    int GetRewriteCharge() const;

    float GetEraseChargeRatio() const;
    float GetRewriteChargeRatio() const;

    std::string GetMatchingCommand(
        const std::string& typed
    ) const;

    std::string GetCompletedCommand(
        const std::string& typed
    ) const;

    void DrawHUD(
    int screenWidth,
    int screenHeight,
    int playerX,
    int playerY,
    bool showQuill,
    const std::string& typed
) const;

private:
    bool stunUnlocked;
    bool freezeUnlocked;
    bool eraseUnlocked;
    bool rewriteUnlocked;

    int stunMaxCharges;
    int freezeMaxCharges;

    int stunCooldowns[2];

    int freezeReadyCharges;
    int freezeChargeProgress;

    int eraseCharge;
    int rewriteCharge;

    float readyPulse;

    static float ClampRatio(float value);
    static std::string Normalize(const std::string& value);

    bool CommandReady(const std::string& command) const;
    int FindReadyStunSlot() const;

    void DrawAbilityLine(
        int x,
        int y,
        const char* name,
        bool unlocked,
        bool ready,
        const std::string& typed,
        const std::string& status
    ) const;

    void DrawChargePips(
        int x,
        int y,
        int readyCharges,
        int maxCharges
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