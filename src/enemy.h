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


    void Update();

    void UpdateDefeat();

    void Draw() const;


    bool CompleteCurrentWord();

    bool HasReachedTarget() const;

    void Defeat();


    const std::string&
    GetCurrentWord() const;


    int GetCurrentWordId() const;

    std::string GetWordDisplay() const;

    int GetWordsRemaining() const;

    int GetStarCount() const;


    std::vector<int>
    GetRemainingWordIds() const;


    bool IsDefeated() const
    {
        return defeated;
    }


    bool HasEscaped() const
    {
        return escaped;
    }


    void SetEscaped(
        bool value
    )
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

    bool IsDefeatFinished() const
{
    return (
        defeated
        &&
        defeatTimer <= 0
    );
}


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


    bool stunned;

    int stunTimer;

    static constexpr int STUN_DURATION = 90;


    float walkPhase;


    static float SpeedValue(
        EnemySpeed type
    );


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