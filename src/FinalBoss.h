#pragma once

#include <string>


class FinalBoss
{
public:
    FinalBoss();

    void Reset();

    void SetViewportSize(
        int width,
        int height
    );

    const std::string& GetCurrentWord() const;
    const std::string& GetReactionText() const;

    // Narrative lines used when the Author steals the player's Quill.
    const char* GetTheftLine(int index) const;
    int GetTheftLineCount() const;

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

    int viewportWidth;
    int viewportHeight;

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

    static constexpr int THEFT_LINE_COUNT = 5;

    static const char* THEFT_LINES[
        THEFT_LINE_COUNT
    ];


    int GetTimerForPhase() const;
};