#pragma once

#include <string>


class FinalBoss
{
public:
    FinalBoss();

    void Reset();

    const std::string& GetCurrentWord() const;
    const std::string& GetReactionText() const;

    int GetPhase() const;
    int GetPhaseCount() const;

    bool CompleteCurrentWord();

    bool IsFinished() const;
    bool IsReacting() const;

    // Attack/timer state.
    bool Update();
    void ResetWordTimer();

    float GetTimerRatio() const;

    bool IsChargingAttack() const;
    bool IsSlashing() const;

    void Draw() const;


private:
    int phase;
    bool finished;

    int hitTimer;
    int reactionTimer;

    int wordTimer;
    int wordTimerMax;

    int slashTimer;

    float pulse;

    std::string reactionText;


    static constexpr int PHASE_COUNT = 4;

    static const char* PHASE_WORDS[
        PHASE_COUNT
    ];

    static const char* PHASE_LABELS[
        PHASE_COUNT
    ];

    static const char* REACTIONS[
        PHASE_COUNT - 1
    ];


    int GetTimerForPhase() const;
};