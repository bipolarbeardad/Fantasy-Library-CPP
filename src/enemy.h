#pragma once

#include "raylib.h"

#include <string>
#include <vector>


enum class EnemyType
{
    Goblin,
    Orc,
    Beast
};


enum class EnemySpeed
{
    Slow,
    Medium,
    Fast
};


class Enemy
{
public:
    Enemy(
        const std::vector<std::string>& words,
        const std::vector<int>& wordIds,
        EnemyType enemyType,
        int lane,
        float startX,
        float startY,
        float targetX,
        float targetY,
        EnemySpeed speedType
    );

    static EnemySpeed RandomSpeed();

    const std::string& GetCurrentWord() const;
    int GetCurrentWordId() const;
    std::string GetWordDisplay() const;

    int GetWordsRemaining() const;
    std::vector<int> GetRemainingWordIds() const;
    int GetStarCount() const;

    bool CompleteCurrentWord();

    void Update();

    // Quill Stun / Freeze support.
    // Unlike the short between-word stagger, this pauses movement
    // without hiding the enemy's current word.
    void ApplyStun(int frames);
    bool IsMovementStunned() const;
    int GetStunFramesRemaining() const;

    bool HasReachedTarget() const;

    void Defeat();
    void UpdateDefeat();

    bool IsDefeated() const
    {
        return defeated;
    }

    bool IsDefeatFinished() const
    {
        return defeated && defeatTimer <= 0;
    }

    bool HasEscaped() const
    {
        return escaped;
    }

    void SetEscaped(bool value)
    {
        escaped = value;
    }

    int GetLane() const
    {
        return lane;
    }

    float GetX() const
    {
        return x;
    }

    float GetY() const
    {
        return y;
    }

    EnemyType GetEnemyType() const
    {
        return enemyType;
    }

    void Draw() const;


private:
    std::vector<std::string> words;
    std::vector<int> wordIds;

    EnemyType enemyType;
    int lane;

    float x;
    float y;
    float targetX;
    float targetY;

    EnemySpeed speedType;
    float speed;

    int currentWordIndex;

    bool defeated;
    int defeatTimer;

    bool escaped;

    // Existing between-word stagger. The next word stays hidden
    // during this brief recovery period.
    bool stunned;
    int stunTimer;

    // Quill-driven movement stun. The word stays visible so the
    // player can continue attacking a frozen creature.
    int movementStunTimer;

    float walkPhase;

    static constexpr int STUN_DURATION = 90;

    static float SpeedValue(EnemySpeed type);

    void DrawStar(
        float centerX,
        float centerY,
        float radius = 7.0f
    ) const;

    void DrawGoblin(
        float drawX,
        float drawY
    ) const;

    void DrawOrc(
        float drawX,
        float drawY
    ) const;

    void DrawBeast(
        float drawX,
        float drawY
    ) const;
};