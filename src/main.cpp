#include "raylib.h"

#include "Combat.h"
#include "Enemy.h"
#include "EnemySprites.h"
#include "FinalBoss.h"
#include "GameFont.h"
#include "MainMenu.h"
#include "MemorySystem.h"
#include "QuillSystem.h"
#include "SaveManager.h"
#include "StoryReader.h"
#include "WordManager.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <utility>
#include <string>
#include <vector>

namespace
{
    constexpr int FPS = 60;
    constexpr int MAX_HEALTH = 3;

    // The entire game is authored against one fixed logical canvas.
    // Window/fullscreen resolution only changes how this canvas is
    // presented, not gameplay geometry or UI placement.
    constexpr int GAME_WIDTH = 900;
    constexpr int GAME_HEIGHT = 600;

    // ID 1125 is reserved exclusively for The Stained Author.
    constexpr int FINAL_WORD_ID = 1125;
    constexpr int FINAL_NORMAL_WORD_COUNT = 1209;

    enum class GameState
    {
        Splash,
        Menu,
        Adventure,
        Story,
        Memory,
        MemoryLibrary,
        ChapterUnlock,
        FinalBossIntro,
        FinalBoss,
        Ending
    };


    struct DisplayOption
    {
        const char* label;
        int width;
        int height;
        bool fullscreen;
    };


    enum class WordSparkPhase
    {
        Attack,
        Return
    };


    struct WordSparkEffect
    {
        Vector2 start;
        Vector2 target;
        Vector2 position;

        WordSparkPhase phase;

        float progress;
        int age;
        int seed;
    };


    constexpr DisplayOption DISPLAY_OPTIONS[] =
    {
        {
            "900 x 600",
            900,
            600,
            false
        },
        {
            "1200 x 800",
            1200,
            800,
            false
        },
        {
            "Fullscreen",
            0,
            0,
            true
        }
    };


    constexpr int DISPLAY_OPTION_COUNT =
        sizeof(DISPLAY_OPTIONS)
        /
        sizeof(DISPLAY_OPTIONS[0]);


    GameState currentState =
        GameState::Splash;


    bool running =
        true;


    MainMenu menu;

    MemorySystem memorySystem;

    QuillSystem quillSystem;

    FinalBoss finalBoss;

    WordManager wordManager;

    Combat combat;

    SaveManager saveManager;

    SaveData saveData;

    Music backgroundMusic = {};
    bool backgroundMusicLoaded = false;


    int splashFrame = 0;

    constexpr int SPLASH_TYPE_START = 20;
    constexpr int SPLASH_FRAMES_PER_LETTER = 5;
    constexpr int SPLASH_GAMING_START = 102;
    constexpr int SPLASH_PRESENTS_START = 132;

    // Presents finishes fading in at frame 160.
    // Hold the completed studio title for one full second
    // before the lightning begins.
    constexpr int SPLASH_LIGHTNING_START = 220;
    constexpr int SPLASH_MENU_FADE_START = 248;
    constexpr int SPLASH_END = 298;


    std::unique_ptr<StoryReader>
        storyReader;


    int displayIndex = 0;

    int musicVolume = 40;


    int playerHealth =
        MAX_HEALTH;

    int healProgress = 0;


    int playerX = 90;

    int playerY = 300;


    std::array<int, 5>
        laneStartY =
        {
            100,
            200,
            300,
            400,
            500
        };


    std::array<int, 5>
        laneTargetY =
        {
            300,
            300,
            300,
            300,
            300
        };


    int waveNumber = 1;

    std::vector<Enemy> enemies;

    std::vector<int>
        reservedWordIds;


    int waveClearTimer = 0;


    std::vector<WordSparkEffect>
        wordSparkEffects;


    int pageRestorePulse = 0;


    enum class ChapterUnlockPhase
    {
        Closing,
        Showing,
        Opening
    };


    int pendingChapterUnlock = -1;

    int chapterUnlockNumber = -1;

    ChapterUnlockPhase chapterUnlockPhase =
        ChapterUnlockPhase::Closing;

    int chapterUnlockFrame = 0;

    constexpr int CHAPTER_CLOSE_FRAMES = 36;

    constexpr int CHAPTER_OPEN_FRAMES = 30;


    int wrongInputFrame = 0;


    // Total enemies still waiting to enter the current wave.
    int queuedEnemies = 0;


    // One active enemy is allowed per lane.
    std::array<bool, 5>
        laneOccupied =
        {
            false,
            false,
            false,
            false,
            false
        };


    // After a lane clears, wait briefly before sending
    // the next queued enemy into that lane.
    std::array<int, 5>
        laneSpawnCooldown =
        {
            0,
            0,
            0,
            0,
            0
        };


    constexpr int LANE_RESPAWN_DELAY =
        42; // 0.7 seconds at 60 FPS


    // Every word assigned to an enemy is unique while
    // that enemy is active. This lets main.cpp remember
    // which lane an enemy belongs to without changing Enemy.h.
    std::vector<std::pair<int, int>>
        wordLaneLookup;


    bool paused = false;

    int pauseSelected = 0;


    bool gameOver = false;

    int gameOverSelected = 0;


    bool memoryReturnsToAdventure = false;

    bool memoryShouldAdvanceWave = false;


    int endingStep = 0;

    int finalBossIntroStep = 0;

    bool finalBossGameOver = false;

    int finalBossGameOverSelected = 0;


    // Forward declarations for helpers used before their
    // full definitions later in this file.
    void ReturnToMainMenu();

    bool CurrentInputIsWrong();


    bool ContainsId(
        const std::vector<int>& ids,
        int id
    )
    {
        return std::find(
            ids.begin(),
            ids.end(),
            id
        )
        !=
        ids.end();
    }


    void AddUniqueId(
        std::vector<int>& ids,
        int id
    )
    {
        if (
            !ContainsId(
                ids,
                id
            )
        )
        {
            ids.push_back(
                id
            );
        }
    }


    void RemoveId(
        std::vector<int>& ids,
        int id
    )
    {
        ids.erase(
            std::remove(
                ids.begin(),
                ids.end(),
                id
            ),
            ids.end()
        );
    }


    void RememberWordLane(
        int wordId,
        int lane
    )
    {
        wordLaneLookup.push_back(
            {
                wordId,
                lane
            }
        );
    }


    int FindLaneForWord(
        int wordId
    )
    {
        for (
            const auto& entry
            :
            wordLaneLookup
        )
        {
            if (entry.first == wordId)
            {
                return entry.second;
            }
        }


        return -1;
    }


    void ForgetLaneWords(
        int lane
    )
    {
        wordLaneLookup.erase(
            std::remove_if(
                wordLaneLookup.begin(),
                wordLaneLookup.end(),
                [lane](
                    const std::pair<int, int>& entry
                )
                {
                    return entry.second == lane;
                }
            ),
            wordLaneLookup.end()
        );
    }


    void ReleaseLane(
        int lane
    )
    {
        if (
            lane < 0
            ||
            lane >= 5
        )
        {
            return;
        }


        laneOccupied[lane] =
            false;


        laneSpawnCooldown[lane] =
            LANE_RESPAWN_DELAY;


        ForgetLaneWords(
            lane
        );
    }


    void ResetWaveQueueState()
    {
        queuedEnemies = 0;

        wordLaneLookup.clear();


        for (
            int lane = 0;
            lane < 5;
            lane++
        )
        {
            laneOccupied[lane] =
                false;


            laneSpawnCooldown[lane] =
                0;
        }
    }


    void RefreshDisplayGeometry()
    {
        // Gameplay geometry never follows the physical window.
        // Everything remains positioned inside the 900x600 canvas.
        const int width =
            GAME_WIDTH;


        const int height =
            GAME_HEIGHT;


        playerX =
            std::max(
                75,
                static_cast<int>(
                    width * 0.10f
                )
            );


        playerY =
            height / 2;


        constexpr float ratios[5] =
        {
            0.17f,
            0.33f,
            0.50f,
            0.67f,
            0.83f
        };


        for (
            int index = 0;
            index < 5;
            index++
        )
        {
            laneStartY[index] =
                static_cast<int>(
                    height
                    *
                    ratios[index]
                );


            laneTargetY[index] =
                playerY;
        }
    }




    void SyncVirtualViewport()
    {
        menu.SetViewportSize(
            GAME_WIDTH,
            GAME_HEIGHT
        );


        memorySystem.SetViewportSize(
            GAME_WIDTH,
            GAME_HEIGHT
        );


        finalBoss.SetViewportSize(
            GAME_WIDTH,
            GAME_HEIGHT
        );


        if (storyReader)
        {
            storyReader->SetViewportSize(
                GAME_WIDTH,
                GAME_HEIGHT
            );
        }
    }


    void DrawVirtualCanvasToWindow(
        const RenderTexture2D& target
    )
    {
        const int windowWidth =
            GetScreenWidth();


        const int windowHeight =
            GetScreenHeight();


        const float scaleX =
            static_cast<float>(
                windowWidth
            )
            /
            static_cast<float>(
                GAME_WIDTH
            );


        const float scaleY =
            static_cast<float>(
                windowHeight
            )
            /
            static_cast<float>(
                GAME_HEIGHT
            );


        const float scale =
            std::min(
                scaleX,
                scaleY
            );


        const float drawWidth =
            static_cast<float>(
                GAME_WIDTH
            )
            *
            scale;


        const float drawHeight =
            static_cast<float>(
                GAME_HEIGHT
            )
            *
            scale;


        const float offsetX =
            (
                static_cast<float>(
                    windowWidth
                )
                -
                drawWidth
            )
            /
            2.0f;


        const float offsetY =
            (
                static_cast<float>(
                    windowHeight
                )
                -
                drawHeight
            )
            /
            2.0f;


        // Anything outside the 3:2 game canvas becomes a clean
        // letterbox/pillarbox instead of stretching the artwork.
        ClearBackground(
            BLACK
        );


        const Rectangle source =
        {
            0.0f,
            0.0f,
            static_cast<float>(
                GAME_WIDTH
            ),
            -static_cast<float>(
                GAME_HEIGHT
            )
        };


        const Rectangle destination =
        {
            offsetX,
            offsetY,
            drawWidth,
            drawHeight
        };


        DrawTexturePro(
            target.texture,
            source,
            destination,
            Vector2{
                0.0f,
                0.0f
            },
            0.0f,
            WHITE
        );
    }


    Vector2 LerpPoint(
        Vector2 start,
        Vector2 target,
        float amount
    )
    {
        return Vector2{
            start.x
            +
            (
                target.x
                -
                start.x
            )
            *
            amount,

            start.y
            +
            (
                target.y
                -
                start.y
            )
            *
            amount
        };
    }


    void LaunchWordSpark(
        const Enemy& enemy
    )
    {
        const Vector2 source =
        {
            static_cast<float>(
                playerX
                +
                (
                    quillSystem.HasQuill()
                    ?
                    34
                    :
                    18
                )
            ),

            static_cast<float>(
                playerY
                -
                (
                    quillSystem.HasQuill()
                    ?
                    18
                    :
                    2
                )
            )
        };


        const Vector2 target =
        {
            enemy.GetX(),
            enemy.GetY()
        };


        wordSparkEffects.push_back(
            WordSparkEffect{
                source,
                target,
                source,
                WordSparkPhase::Attack,
                0.0f,
                0,
                GetRandomValue(
                    0,
                    10000
                )
            }
        );
    }


    Vector2 GetWordSparkPosition(
        const WordSparkEffect& spark,
        float amount
    )
    {
        const float t =
            std::clamp(
                amount,
                0.0f,
                1.0f
            );


        const float eased =
            t
            *
            t
            *
            (
                3.0f
                -
                2.0f
                *
                t
            );


        if (
            spark.phase
            ==
            WordSparkPhase::Attack
        )
        {
            return LerpPoint(
                spark.start,
                spark.target,
                eased
            );
        }


        // Returning words take a magical curved route instead of
        // flying straight at the player like a projectile.
        const Vector2 midpoint =
        {
            (
                spark.start.x
                +
                spark.target.x
            )
            *
            0.5f,

            (
                spark.start.y
                +
                spark.target.y
            )
            *
            0.5f
            -
            72.0f
        };


        const float inverse =
            1.0f
            -
            eased;


        Vector2 position =
        {
            inverse
            *
            inverse
            *
            spark.start.x
            +
            2.0f
            *
            inverse
            *
            eased
            *
            midpoint.x
            +
            eased
            *
            eased
            *
            spark.target.x,

            inverse
            *
            inverse
            *
            spark.start.y
            +
            2.0f
            *
            inverse
            *
            eased
            *
            midpoint.y
            +
            eased
            *
            eased
            *
            spark.target.y
        };


        // One shrinking flourish makes the effect read as a word
        // spiraling home rather than an enemy firing an arrow.
        const float flourishRadius =
            42.0f
            *
            std::sin(
                PI
                *
                eased
            )
            *
            (
                1.0f
                -
                eased
                *
                0.45f
            );


        const float flourishAngle =
            eased
            *
            PI
            *
            2.35f
            +
            static_cast<float>(
                spark.seed % 11
            )
            *
            0.19f;


        position.x +=
            std::cos(
                flourishAngle
            )
            *
            flourishRadius;


        position.y +=
            std::sin(
                flourishAngle
            )
            *
            flourishRadius
            *
            0.62f;


        return position;
    }


    void UpdateWordSparks()
    {
        for (
            WordSparkEffect& spark
            :
            wordSparkEffects
        )
        {
            spark.age++;


            const float speed =
                spark.phase
                ==
                WordSparkPhase::Attack
                ?
                0.095f
                :
                0.043f;


            spark.progress =
                std::min(
                    1.0f,
                    spark.progress
                    +
                    speed
                );


            spark.position =
                GetWordSparkPosition(
                    spark,
                    spark.progress
                );


            if (spark.progress < 1.0f)
            {
                continue;
            }


            if (
                spark.phase
                ==
                WordSparkPhase::Attack
            )
            {
                // Every completed word is permanently recovered,
                // even when the creature still has more words.
                // Send that recovered fragment back immediately.
                spark.phase =
                    WordSparkPhase::Return;


                spark.start =
                    spark.target;


                spark.target =
                    Vector2{
                        static_cast<float>(
                            playerX + 3
                        ),
                        static_cast<float>(
                            playerY + 12
                        )
                    };


                spark.position =
                    spark.start;


                spark.progress =
                    0.0f;


                spark.age =
                    0;
            }

            else
            {
                // The permanent restoration already happened when
                // the word was saved. This pulse makes its arrival
                // visibly affect the parchment.
                pageRestorePulse =
                    18;
            }
        }


        wordSparkEffects.erase(
            std::remove_if(
                wordSparkEffects.begin(),
                wordSparkEffects.end(),
                [](
                    const WordSparkEffect& spark
                )
                {
                    return
                        spark.phase
                        ==
                        WordSparkPhase::Return
                        &&
                        spark.progress
                        >=
                        1.0f;
                }
            ),
            wordSparkEffects.end()
        );
    }


    void DrawWordSparks()
    {
        for (
            const WordSparkEffect& spark
            :
            wordSparkEffects
        )
        {
            const Color core =
                spark.phase
                ==
                WordSparkPhase::Attack
                ?
                Color{
                    255,
                    235,
                    125,
                    255
                }
                :
                Color{
                    238,
                    210,
                    145,
                    255
                };


            const Color trail =
                spark.phase
                ==
                WordSparkPhase::Attack
                ?
                Color{
                    255,
                    210,
                    75,
                    145
                }
                :
                Color{
                    210,
                    180,
                    115,
                    125
                };


            const Vector2 previous =
                GetWordSparkPosition(
                    spark,
                    std::max(
                        0.0f,
                        spark.progress
                        -
                        (
                            spark.phase
                            ==
                            WordSparkPhase::Return
                            ?
                            0.035f
                            :
                            0.09f
                        )
                    )
                );


            DrawLineEx(
                previous,
                spark.position,
                spark.phase
                ==
                WordSparkPhase::Return
                ?
                3.5f
                :
                2.0f,
                trail
            );


            if (
                spark.phase
                ==
                WordSparkPhase::Return
            )
            {
                const float sparkleSize =
                    5.0f
                    +
                    2.0f
                    *
                    std::sin(
                        static_cast<float>(
                            spark.age
                        )
                        *
                        0.55f
                    );


                DrawLineEx(
                    Vector2{
                        spark.position.x
                        -
                        sparkleSize,
                        spark.position.y
                    },
                    Vector2{
                        spark.position.x
                        +
                        sparkleSize,
                        spark.position.y
                    },
                    2.0f,
                    Color{
                        255,
                        239,
                        170,
                        220
                    }
                );


                DrawLineEx(
                    Vector2{
                        spark.position.x,
                        spark.position.y
                        -
                        sparkleSize
                    },
                    Vector2{
                        spark.position.x,
                        spark.position.y
                        +
                        sparkleSize
                    },
                    2.0f,
                    Color{
                        255,
                        239,
                        170,
                        220
                    }
                );
            }


            DrawCircleV(
                spark.position,
                4.0f,
                core
            );


            DrawCircleV(
                spark.position,
                7.0f,
                Color{
                    core.r,
                    core.g,
                    core.b,
                    55
                }
            );


            // Three tiny orbiting flecks create a sparkle without
            // requiring another sprite or particle-system file.
            const int fleckCount =
                spark.phase
                ==
                WordSparkPhase::Return
                ?
                6
                :
                3;


            for (
                int index = 0;
                index < fleckCount;
                index++
            )
            {
                const float angle =
                    static_cast<float>(
                        spark.age
                    )
                    *
                    0.35f
                    +
                    static_cast<float>(
                        index
                    )
                    *
                    2.094f
                    +
                    static_cast<float>(
                        spark.seed % 17
                    );


                const float radius =
                    6.0f
                    +
                    static_cast<float>(
                        index
                    );


                DrawCircle(
                    static_cast<int>(
                        spark.position.x
                        +
                        std::cos(
                            angle
                        )
                        *
                        radius
                    ),
                    static_cast<int>(
                        spark.position.y
                        +
                        std::sin(
                            angle
                        )
                        *
                        radius
                    ),
                    1.5f,
                    core
                );
            }
        }
    }


    unsigned char SplashAlpha(
        float value
    )
    {
        return static_cast<unsigned char>(
            std::clamp(
                value,
                0.0f,
                255.0f
            )
        );
    }


    void DrawStudioLightning(
        float reveal,
        unsigned char alpha
    )
    {
        const Vector2 points[] =
        {
            // The bolt begins just beneath "Presents" and
            // travels downward so it never covers the studio name.
            { 450.0f, 372.0f },
            { 438.0f, 405.0f },
            { 456.0f, 405.0f },
            { 434.0f, 448.0f },
            { 449.0f, 448.0f },
            { 428.0f, 500.0f },
            { 462.0f, 456.0f },
            { 446.0f, 456.0f },
            { 470.0f, 413.0f },
            { 453.0f, 413.0f },
            { 475.0f, 372.0f }
        };


        constexpr int pointCount =
            sizeof(points)
            /
            sizeof(points[0]);


        const float segmentProgress =
            std::clamp(
                reveal,
                0.0f,
                1.0f
            )
            *
            static_cast<float>(
                pointCount - 1
            );


        for (
            int index = 0;
            index < pointCount - 1;
            index++
        )
        {
            const float local =
                std::clamp(
                    segmentProgress
                    -
                    static_cast<float>(
                        index
                    ),
                    0.0f,
                    1.0f
                );


            if (local <= 0.0f)
            {
                break;
            }


            const Vector2 end =
            {
                points[index].x
                +
                (
                    points[index + 1].x
                    -
                    points[index].x
                )
                *
                local,

                points[index].y
                +
                (
                    points[index + 1].y
                    -
                    points[index].y
                )
                *
                local
            };


            DrawLineEx(
                points[index],
                end,
                10.0f,
                Color{
                    42,
                    5,
                    65,
                    SplashAlpha(
                        alpha * 0.55f
                    )
                }
            );


            DrawLineEx(
                points[index],
                end,
                5.0f,
                Color{
                    132,
                    42,
                    185,
                    alpha
                }
            );


            DrawLineEx(
                points[index],
                end,
                2.0f,
                Color{
                    242,
                    205,
                    255,
                    alpha
                }
            );
        }


        // Small red accents give the studio mark its
        // red-and-purple identity without another asset.

        // Top cap closes the opening of the studio bolt.
        DrawLineEx(
            Vector2{
                450.0f,
                372.0f
            },
            Vector2{
                475.0f,
                372.0f
            },
            3.0f,
            Color{
                190,
                52,
                86,
                SplashAlpha(
                    alpha * reveal
                )
            }
        );


        DrawLineEx(
            Vector2{
                440.0f,
                405.0f
            },
            Vector2{
                456.0f,
                405.0f
            },
            3.0f,
            Color{
                190,
                52,
                86,
                SplashAlpha(
                    alpha * reveal
                )
            }
        );


        DrawLineEx(
            Vector2{
                436.0f,
                448.0f
            },
            Vector2{
                452.0f,
                448.0f
            },
            3.0f,
            Color{
                190,
                52,
                86,
                SplashAlpha(
                    alpha * reveal
                )
            }
        );
    }


    void DrawSplash()
    {
        const std::string studioName =
            "Bearly Bipolar";


        if (splashFrame >= SPLASH_MENU_FADE_START)
        {
            menu.Draw();


            const float fadeProgress =
                std::clamp(
                    static_cast<float>(
                        splashFrame
                        -
                        SPLASH_MENU_FADE_START
                    )
                    /
                    static_cast<float>(
                        SPLASH_END
                        -
                        SPLASH_MENU_FADE_START
                    ),
                    0.0f,
                    1.0f
                );


            DrawRectangle(
                0,
                0,
                GAME_WIDTH,
                GAME_HEIGHT,
                Color{
                    5,
                    2,
                    8,
                    SplashAlpha(
                        255.0f
                        *
                        (
                            1.0f
                            -
                            fadeProgress
                        )
                    )
                }
            );


            const float lightningFade =
                1.0f
                -
                fadeProgress;


            DrawStudioLightning(
                1.0f,
                SplashAlpha(
                    255.0f
                    *
                    lightningFade
                )
            );


            return;
        }


        ClearBackground(
            Color{
                5,
                2,
                8,
                255
            }
        );


        const int typedCount =
            std::clamp(
                (
                    splashFrame
                    -
                    SPLASH_TYPE_START
                )
                /
                SPLASH_FRAMES_PER_LETTER,
                0,
                static_cast<int>(
                    studioName.size()
                )
            );


        const std::string typed =
            studioName.substr(
                0,
                typedCount
            );


        const int titleFontSize = 48;
        const int titleY = 205;


        const int fullTitleWidth =
            MeasureGameText(
                studioName,
                titleFontSize
            );


        const int titleX =
            GAME_WIDTH / 2
            -
            fullTitleWidth / 2;


        DrawGameText(
            typed,
            titleX,
            titleY,
            titleFontSize,
            Color{
                174,
                68,
                190,
                255
            }
        );


        if (
            typedCount
            <
            static_cast<int>(
                studioName.size()
            )
            &&
            splashFrame
            >=
            SPLASH_TYPE_START
            &&
            (
                splashFrame / 18
            )
            %
            2
            ==
            0
        )
        {
            const int cursorX =
                titleX
                +
                MeasureGameText(
                    typed,
                    titleFontSize
                );


            DrawRectangle(
                cursorX + 3,
                titleY + 5,
                3,
                titleFontSize - 8,
                Color{
                    220,
                    105,
                    145,
                    230
                }
            );
        }


        if (splashFrame >= SPLASH_GAMING_START)
        {
            const float slideProgress =
                std::clamp(
                    static_cast<float>(
                        splashFrame
                        -
                        SPLASH_GAMING_START
                    )
                    /
                    24.0f,
                    0.0f,
                    1.0f
                );


            const float eased =
                1.0f
                -
                std::pow(
                    1.0f
                    -
                    slideProgress,
                    3.0f
                );


            const char* gaming =
                "Gaming";


            const int gamingFontSize = 37;


            const int gamingX =
                GAME_WIDTH / 2
                -
                MeasureGameText(
                    gaming,
                    gamingFontSize
                )
                /
                2;


            const int gamingY =
                static_cast<int>(
                    325.0f
                    -
                    62.0f
                    *
                    eased
                );


            DrawGameText(
                gaming,
                gamingX,
                gamingY,
                gamingFontSize,
                Color{
                    205,
                    63,
                    92,
                    SplashAlpha(
                        255.0f
                        *
                        slideProgress
                    )
                }
            );
        }


        if (splashFrame >= SPLASH_PRESENTS_START)
        {
            const float presentsProgress =
                std::clamp(
                    static_cast<float>(
                        splashFrame
                        -
                        SPLASH_PRESENTS_START
                    )
                    /
                    28.0f,
                    0.0f,
                    1.0f
                );


            const char* presents =
                "Presents";


            const int presentsFontSize = 24;


            DrawGameText(
                presents,
                GAME_WIDTH / 2
                -
                MeasureGameText(
                    presents,
                    presentsFontSize
                )
                /
                2,
                335,
                presentsFontSize,
                Color{
                    220,
                    205,
                    220,
                    SplashAlpha(
                        255.0f
                        *
                        presentsProgress
                    )
                }
            );
        }


        if (splashFrame >= SPLASH_LIGHTNING_START)
        {
            const float reveal =
                std::clamp(
                    static_cast<float>(
                        splashFrame
                        -
                        SPLASH_LIGHTNING_START
                    )
                    /
                    22.0f,
                    0.0f,
                    1.0f
                );


            DrawStudioLightning(
                reveal,
                255
            );
        }
    }


    void UpdateSplash()
    {
        splashFrame++;


        if (
            IsKeyPressed(KEY_ENTER)
            ||
            IsKeyPressed(KEY_SPACE)
            ||
            IsKeyPressed(KEY_ESCAPE)
            ||
            splashFrame >= SPLASH_END
        )
        {
            splashFrame =
                SPLASH_END;


            currentState =
                GameState::Menu;
        }
    }


    void SaveProgress()
    {
        saveData.wordsRecovered =
            static_cast<int>(
                saveData
                .recoveredWordIds
                .size()
            );


        saveData.displayIndex =
            displayIndex;


        saveData.musicVolume =
            musicVolume;


        saveManager.SaveGame(
            saveData
        );
    }


    void ApplyDisplaySetting()
    {
        displayIndex =
            std::clamp(
                displayIndex,
                0,
                DISPLAY_OPTION_COUNT - 1
            );


        const DisplayOption& option =
            DISPLAY_OPTIONS[
                displayIndex
            ];


        const bool currentlyFullscreen =
            IsWindowFullscreen();


        if (option.fullscreen)
        {
            if (!currentlyFullscreen)
            {
                ToggleFullscreen();
            }
        }

        else
        {
            if (currentlyFullscreen)
            {
                ToggleFullscreen();
            }


            SetWindowSize(
                option.width,
                option.height
            );


            const int monitor =
                GetCurrentMonitor();


            const int monitorWidth =
                GetMonitorWidth(
                    monitor
                );


            const int monitorHeight =
                GetMonitorHeight(
                    monitor
                );


            SetWindowPosition(
                std::max(
                    0,
                    (
                        monitorWidth
                        -
                        option.width
                    )
                    /
                    2
                ),
                std::max(
                    0,
                    (
                        monitorHeight
                        -
                        option.height
                    )
                    /
                    2
                )
            );
        }


        RefreshDisplayGeometry();


        menu.SetSettings(
            option.label,
            musicVolume
        );
    }


    void DrawHeart(
        int x,
        int y,
        bool filled
    )
    {
        const Color color =
            filled
            ?
            Color{
                220,
                55,
                75,
                255
            }
            :
            Color{
                70,
                70,
                80,
                255
            };


        DrawCircle(
            x - 6,
            y,
            7,
            color
        );


        DrawCircle(
            x + 6,
            y,
            7,
            color
        );


        // Bottom point of the heart.
        // This vertex order is correct for raylib.
        DrawTriangle(
            Vector2{
                static_cast<float>(
                    x - 13
                ),
                static_cast<float>(
                    y + 1
                )
            },

            Vector2{
                static_cast<float>(
                    x
                ),
                static_cast<float>(
                    y + 18
                )
            },

            Vector2{
                static_cast<float>(
                    x + 13
                ),
                static_cast<float>(
                    y + 1
                )
            },

            color
        );
    }


    void DrawPlayer()
    {
        const Color robe =
        {
            90,
            110,
            190,
            255
        };


        const Color robeDark =
        {
            52,
            66,
            135,
            255
        };


        const Color pages =
        {
            238,
            224,
            155,
            255
        };


        const Color pageEdge =
        {
            165,
            145,
            82,
            255
        };


        const Color skin =
        {
            225,
            200,
            165,
            255
        };


        // Shadow.
        DrawEllipse(
            playerX,
            playerY + 36,
            24,
            6,
            Color{
                8,
                8,
                15,
                255
            }
        );


        // Robe.
        DrawTriangle(
            Vector2{
                static_cast<float>(
                    playerX - 16
                ),
                static_cast<float>(
                    playerY - 6
                )
            },

            Vector2{
                static_cast<float>(
                    playerX
                ),
                static_cast<float>(
                    playerY + 34
                )
            },

            Vector2{
                static_cast<float>(
                    playerX + 16
                ),
                static_cast<float>(
                    playerY - 6
                )
            },

            robe
        );


        DrawTriangle(
            Vector2{
                static_cast<float>(
                    playerX - 16
                ),
                static_cast<float>(
                    playerY - 6
                )
            },

            Vector2{
                static_cast<float>(
                    playerX - 20
                ),
                static_cast<float>(
                    playerY + 31
                )
            },

            Vector2{
                static_cast<float>(
                    playerX
                ),
                static_cast<float>(
                    playerY + 34
                )
            },

            robeDark
        );


        DrawTriangle(
            Vector2{
                static_cast<float>(
                    playerX + 16
                ),
                static_cast<float>(
                    playerY - 6
                )
            },

            Vector2{
                static_cast<float>(
                    playerX
                ),
                static_cast<float>(
                    playerY + 34
                )
            },

            Vector2{
                static_cast<float>(
                    playerX + 20
                ),
                static_cast<float>(
                    playerY + 31
                )
            },

            robeDark
        );


        // Head.
        DrawCircle(
            playerX,
            playerY - 23,
            12,
            skin
        );


        // Hair/cap.
        DrawCircleSector(
            Vector2{
                static_cast<float>(
                    playerX
                ),
                static_cast<float>(
                    playerY - 25
                )
            },
            13.0f,
            180.0f,
            360.0f,
            16,
            Color{
                230,
                220,
                150,
                255
            }
        );


        // Left page.
        DrawTriangle(
            Vector2{
                static_cast<float>(
                    playerX - 23
                ),
                static_cast<float>(
                    playerY + 1
                )
            },

            Vector2{
                static_cast<float>(
                    playerX - 2
                ),
                static_cast<float>(
                    playerY + 18
                )
            },

            Vector2{
                static_cast<float>(
                    playerX - 2
                ),
                static_cast<float>(
                    playerY + 7
                )
            },

            pages
        );


        DrawTriangle(
            Vector2{
                static_cast<float>(
                    playerX - 23
                ),
                static_cast<float>(
                    playerY + 1
                )
            },

            Vector2{
                static_cast<float>(
                    playerX - 23
                ),
                static_cast<float>(
                    playerY + 14
                )
            },

            Vector2{
                static_cast<float>(
                    playerX - 2
                ),
                static_cast<float>(
                    playerY + 18
                )
            },

            pages
        );


        // Right page.
        DrawTriangle(
            Vector2{
                static_cast<float>(
                    playerX + 2
                ),
                static_cast<float>(
                    playerY + 7
                )
            },

            Vector2{
                static_cast<float>(
                    playerX + 2
                ),
                static_cast<float>(
                    playerY + 18
                )
            },

            Vector2{
                static_cast<float>(
                    playerX + 23
                ),
                static_cast<float>(
                    playerY + 1
                )
            },

            pages
        );


        DrawTriangle(
            Vector2{
                static_cast<float>(
                    playerX + 23
                ),
                static_cast<float>(
                    playerY + 1
                )
            },

            Vector2{
                static_cast<float>(
                    playerX + 2
                ),
                static_cast<float>(
                    playerY + 18
                )
            },

            Vector2{
                static_cast<float>(
                    playerX + 23
                ),
                static_cast<float>(
                    playerY + 14
                )
            },

            pages
        );


        // Book seam.
        DrawLineEx(
            Vector2{
                static_cast<float>(
                    playerX
                ),
                static_cast<float>(
                    playerY + 6
                )
            },

            Vector2{
                static_cast<float>(
                    playerX
                ),
                static_cast<float>(
                    playerY + 19
                )
            },

            2.0f,
            pageEdge
        );


        // Once Memory II restores the Quill, the Word Seeker
        // visibly carries it during normal play. During the final
        // confrontation the Stained Author has stolen it.
        const bool authorHasQuill =
            currentState
            ==
            GameState::FinalBossIntro
            ||
            currentState
            ==
            GameState::FinalBoss;


        if (
            quillSystem.HasQuill()
            &&
            !authorHasQuill
        )
        {
            const Vector2 hand =
            {
                static_cast<float>(
                    playerX + 17
                ),
                static_cast<float>(
                    playerY + 6
                )
            };


            const Vector2 featherBase =
            {
                static_cast<float>(
                    playerX + 38
                ),
                static_cast<float>(
                    playerY - 23
                )
            };


            DrawLineEx(
                hand,
                featherBase,
                3.0f,
                Color{
                    215,
                    195,
                    145,
                    255
                }
            );


            DrawTriangle(
                Vector2{
                    static_cast<float>(
                        playerX + 36
                    ),
                    static_cast<float>(
                        playerY - 20
                    )
                },
                Vector2{
                    static_cast<float>(
                        playerX + 49
                    ),
                    static_cast<float>(
                        playerY - 39
                    )
                },
                Vector2{
                    static_cast<float>(
                        playerX + 42
                    ),
                    static_cast<float>(
                        playerY - 15
                    )
                },
                Color{
                    235,
                    225,
                    205,
                    255
                }
            );


            DrawTriangle(
                Vector2{
                    static_cast<float>(
                        playerX + 37
                    ),
                    static_cast<float>(
                        playerY - 21
                    )
                },
                Vector2{
                    static_cast<float>(
                        playerX + 29
                    ),
                    static_cast<float>(
                        playerY - 30
                    )
                },
                Vector2{
                    static_cast<float>(
                        playerX + 42
                    ),
                    static_cast<float>(
                        playerY - 15
                    )
                },
                Color{
                    220,
                    210,
                    195,
                    255
                }
            );
        }
    }


    int GetWaveSize()
    {
        const int recovered =
            saveData.wordsRecovered;


        // Early game stays gentle.
        if (recovered < 25)
        {
            return GetRandomValue(
                2,
                3
            );
        }


        if (recovered < 75)
        {
            return GetRandomValue(
                2,
                4
            );
        }


        if (recovered < 250)
        {
            return GetRandomValue(
                2,
                5
            );
        }


        if (recovered < 500)
        {
            return GetRandomValue(
                3,
                6
            );
        }


        if (recovered < 750)
        {
            return GetRandomValue(
                3,
                7
            );
        }


        if (recovered < 900)
        {
            return GetRandomValue(
                4,
                8
            );
        }


        if (recovered < 1050)
        {
            return GetRandomValue(
                5,
                9
            );
        }


        if (recovered < 1100)
        {
            return GetRandomValue(
                6,
                9
            );
        }


        // Late game can contain as many as ten enemies,
        // but never more than five are active at once.
        return GetRandomValue(
            6,
            10
        );
    }


    int GetEnemyWordCount()
    {
        const int recovered =
            saveData.wordsRecovered;


        if (recovered < 25)
        {
            return 1;
        }


        const int roll =
            GetRandomValue(
                1,
                100
            );


        if (recovered < 75)
        {
            return roll <= 30
                ?
                2
                :
                1;
        }


        if (recovered < 200)
        {
            if (roll <= 15)
            {
                return 3;
            }


            if (roll <= 50)
            {
                return 2;
            }


            return 1;
        }


        if (roll <= 30)
        {
            return 3;
        }


        if (roll <= 70)
        {
            return 2;
        }


        return 1;
    }


    std::vector<WordRecord>
    GetWordRecords(
        int count,
        EnemyType enemyType
    )
    {
        std::vector<WordRecord>
            selected;


        for (
            int index = 0;
            index < count;
            index++
        )
        {
            std::vector<int>
                excluded =
                    saveData
                    .recoveredWordIds;


            // TheEnd belongs only to The Stained Author.
            AddUniqueId(
                excluded,
                FINAL_WORD_ID
            );


            for (
                int id
                :
                reservedWordIds
            )
            {
                AddUniqueId(
                    excluded,
                    id
                );
            }


            for (
                const WordRecord& record
                :
                selected
            )
            {
                AddUniqueId(
                    excluded,
                    record.id
                );
            }


            const std::vector<WordRecord>
                records =
                    wordManager
                    .GetWordsForChapter(
                        excluded,
                        enemyType,
                        wordManager.GetCurrentChapter(
                            saveData.recoveredWordIds
                        )
                    );


            if (records.empty())
            {
                break;
            }


            selected.push_back(
                records.front()
            );
        }


        return selected;
    }


    bool CreateEnemy(
        int laneIndex
    )
    {
        const int wordCount =
            GetEnemyWordCount();


        const EnemyType enemyType =
            wordManager.GetEnemyType(
                saveData.wordsRecovered
            );


        const std::vector<WordRecord>
            records =
                GetWordRecords(
                    wordCount,
                    enemyType
                );


        if (records.empty())
        {
            return false;
        }


        std::vector<std::string>
            words;


        std::vector<int>
            wordIds;


        for (
            const WordRecord& record
            :
            records
        )
        {
            words.push_back(
                record.word
            );


            wordIds.push_back(
                record.id
            );


            AddUniqueId(
                reservedWordIds,
                record.id
            );


            RememberWordLane(
                record.id,
                laneIndex
            );
        }


        enemies.emplace_back(
            words,
            wordIds,
            enemyType,
            laneIndex,
            static_cast<float>(
                GAME_WIDTH
                +
                70
            ),
            static_cast<float>(
                laneStartY[
                    laneIndex
                ]
            ),
            static_cast<float>(
                playerX
            ),
            static_cast<float>(
                laneTargetY[
                    laneIndex
                ]
            ),
            Enemy::RandomSpeed()
        );


        laneOccupied[laneIndex] =
            true;


        return true;
    }


    bool SpawnQueuedEnemyInLane(
        int laneIndex
    )
    {
        if (
            queuedEnemies <= 0
            ||
            laneOccupied[laneIndex]
            ||
            laneSpawnCooldown[laneIndex] > 0
        )
        {
            return false;
        }


        if (
            CreateEnemy(
                laneIndex
            )
        )
        {
            queuedEnemies--;

            return true;
        }


        // No unrecovered words remain to assign.
        queuedEnemies = 0;

        return false;
    }


    void UpdateQueuedSpawns()
    {
        for (
            int lane = 0;
            lane < 5;
            lane++
        )
        {
            if (laneSpawnCooldown[lane] > 0)
            {
                laneSpawnCooldown[lane]--;
            }
        }


        if (queuedEnemies <= 0)
        {
            return;
        }


        std::array<int, 5> lanes =
        {
            0,
            1,
            2,
            3,
            4
        };


        // Shuffle available lane order so replacements
        // do not always favor the top lane.
        for (
            int index = 4;
            index > 0;
            index--
        )
        {
            const int other =
                GetRandomValue(
                    0,
                    index
                );


            std::swap(
                lanes[index],
                lanes[other]
            );
        }


        for (
            int lane
            :
            lanes
        )
        {
            if (queuedEnemies <= 0)
            {
                break;
            }


            SpawnQueuedEnemyInLane(
                lane
            );
        }
    }


    void StartWave()
    {
        enemies.clear();

        reservedWordIds.clear();

        wordSparkEffects.clear();

        pageRestorePulse = 0;

        wrongInputFrame = 0;

        combat.ClearInput();

        waveClearTimer = 0;


        ResetWaveQueueState();


        queuedEnemies =
            GetWaveSize();


        std::array<int, 5> lanes =
        {
            0,
            1,
            2,
            3,
            4
        };


        // Randomize which lanes receive the opening enemies.
        for (
            int index = 4;
            index > 0;
            index--
        )
        {
            const int other =
                GetRandomValue(
                    0,
                    index
                );


            std::swap(
                lanes[index],
                lanes[other]
            );
        }


        // Fill at most five lanes. Any remaining enemies
        // stay queued until one of those lanes becomes free.
        for (
            int lane
            :
            lanes
        )
        {
            if (queuedEnemies <= 0)
            {
                break;
            }


            if (
                CreateEnemy(
                    lane
                )
            )
            {
                queuedEnemies--;
            }

            else
            {
                queuedEnemies = 0;

                break;
            }
        }
    }


    void RecoverWord(
        int wordId
    )
    {
        if (
            ContainsId(
                saveData.recoveredWordIds,
                wordId
            )
        )
        {
            return;
        }


        const int chapterBeforeRecovery =
            wordManager.GetCurrentChapter(
                saveData.recoveredWordIds
            );


        saveData
            .recoveredWordIds
            .push_back(
                wordId
            );


        RemoveId(
            reservedWordIds,
            wordId
        );


        saveData.wordsRecovered =
            static_cast<int>(
                saveData
                .recoveredWordIds
                .size()
            );


        quillSystem.SyncUnlocks(
            saveData.wordsRecovered
        );


        // Every newly recovered unique word charges the
        // word-powered Quill abilities.
        quillSystem.OnWordsRecovered(
            1
        );


        if (
            chapterBeforeRecovery > 0
            &&
            wordManager.IsChapterComplete(
                chapterBeforeRecovery,
                saveData.recoveredWordIds
            )
        )
        {
            pendingChapterUnlock =
                chapterBeforeRecovery;
        }


        if (
            playerHealth
            <
            MAX_HEALTH
        )
        {
            healProgress++;


            if (
                healProgress
                >=
                10
            )
            {
                playerHealth++;

                healProgress -= 10;


                if (
                    playerHealth
                    >=
                    MAX_HEALTH
                )
                {
                    playerHealth =
                        MAX_HEALTH;


                    healProgress = 0;
                }
            }
        }


        SaveProgress();


        if (storyReader)
        {
            storyReader
                ->SetRecoveredWords(
                    saveData
                    .recoveredWordIds
                );
        }
    }


    Enemy* GetMostDangerousEnemy()
    {
        Enemy* best =
            nullptr;


        float bestDistance =
            1000000000.0f;


        for (
            Enemy& enemy
            :
            enemies
        )
        {
            if (
                enemy.IsDefeated()
                ||
                enemy.HasEscaped()
            )
            {
                continue;
            }


            const float dx =
                enemy.GetX()
                -
                static_cast<float>(
                    playerX
                );


            const float dy =
                enemy.GetY()
                -
                static_cast<float>(
                    playerY
                );


            const float distance =
                dx * dx
                +
                dy * dy;


            if (
                best == nullptr
                ||
                distance < bestDistance
            )
            {
                best =
                    &enemy;


                bestDistance =
                    distance;
            }
        }


        return best;
    }


    void EraseEnemy(
        Enemy& enemy
    )
    {
        if (
            enemy.IsDefeated()
            ||
            enemy.HasEscaped()
        )
        {
            return;
        }


        const std::vector<int>
            remainingIds =
                enemy.GetRemainingWordIds();


        int lane =
            enemy.GetLane();


        for (
            int wordId
            :
            remainingIds
        )
        {
            // TheEnd can only be restored by the Stained Author.
            if (wordId == FINAL_WORD_ID)
            {
                continue;
            }


            RecoverWord(
                wordId
            );


            RemoveId(
                reservedWordIds,
                wordId
            );
        }


        enemy.Defeat();


        ReleaseLane(
            lane
        );


        combat.ClearInput();
    }


    void UseStunAbility()
    {
        if (!quillSystem.IsStunReady())
        {
            return;
        }


        Enemy* target =
            GetMostDangerousEnemy();


        if (target == nullptr)
        {
            return;
        }


        if (quillSystem.UseStun())
        {
            target->ApplyStun(
                QuillSystem::STUN_DURATION_FRAMES
            );
        }
    }


    void UseFreezeAbility()
    {
        if (!quillSystem.IsFreezeReady())
        {
            return;
        }


        bool foundTarget =
            false;


        for (
            const Enemy& enemy
            :
            enemies
        )
        {
            if (
                !enemy.IsDefeated()
                &&
                !enemy.HasEscaped()
            )
            {
                foundTarget =
                    true;

                break;
            }
        }


        if (!foundTarget)
        {
            return;
        }


        if (!quillSystem.UseFreeze())
        {
            return;
        }


        for (
            Enemy& enemy
            :
            enemies
        )
        {
            if (
                enemy.IsDefeated()
                ||
                enemy.HasEscaped()
            )
            {
                continue;
            }


            enemy.ApplyStun(
                QuillSystem::FREEZE_DURATION_FRAMES
            );
        }


        combat.ClearInput();
    }


    void UseEraseAbility()
    {
        if (!quillSystem.IsEraseReady())
        {
            return;
        }


        Enemy* target =
            GetMostDangerousEnemy();


        if (target == nullptr)
        {
            return;
        }


        if (!quillSystem.UseErase())
        {
            return;
        }


        EraseEnemy(
            *target
        );
    }


    void UseRewriteAbility()
    {
        if (!quillSystem.IsRewriteReady())
        {
            return;
        }


        bool foundTarget =
            false;


        for (
            const Enemy& enemy
            :
            enemies
        )
        {
            if (
                !enemy.IsDefeated()
                &&
                !enemy.HasEscaped()
            )
            {
                foundTarget =
                    true;

                break;
            }
        }


        if (!foundTarget)
        {
            return;
        }


        if (!quillSystem.UseRewrite())
        {
            return;
        }


        // Rewrite affects only creatures already present on the
        // board. Queued creatures have not yet stolen any words.
        for (
            Enemy& enemy
            :
            enemies
        )
        {
            if (
                enemy.IsDefeated()
                ||
                enemy.HasEscaped()
            )
            {
                continue;
            }


            EraseEnemy(
                enemy
            );
        }


        combat.ClearInput();
    }


    void EnemyReachedPlayer(
        Enemy& enemy
    )
    {
        int lane =
            -1;


        const std::vector<int>
            remainingWordIds =
                enemy.GetRemainingWordIds();


        if (!remainingWordIds.empty())
        {
            lane =
                FindLaneForWord(
                    remainingWordIds.front()
                );
        }


        int damage =
            enemy.GetWordsRemaining();


        if (damage <= 0)
        {
            damage = 1;
        }


        const bool wasFullHealth =
            playerHealth
            ==
            MAX_HEALTH;


        playerHealth -=
            damage;


        if (
            wasFullHealth
            &&
            playerHealth
            <
            MAX_HEALTH
        )
        {
            healProgress = 0;
        }


        for (
            int wordId
            :
            enemy
            .GetRemainingWordIds()
        )
        {
            RemoveId(
                reservedWordIds,
                wordId
            );
        }


        enemy.SetEscaped(
            true
        );


        ReleaseLane(
            lane
        );


        combat.ClearInput();


        if (playerHealth <= 0)
        {
            playerHealth = 0;

            gameOver = true;

            gameOverSelected = 0;
        }
    }


    std::string NormalizeTypingText(
        const std::string& value
    );


    void StartFinalBoss()
    {
        enemies.clear();

        reservedWordIds.clear();

        ResetWaveQueueState();

        combat.ClearInput();


        finalBoss.Reset();

        playerHealth =
            MAX_HEALTH;

        healProgress =
            0;

        finalBossGameOver =
            false;

        finalBossGameOverSelected =
            0;

        finalBossIntroStep = 0;

        currentState =
            GameState::FinalBossIntro;
    }


    void BeginEnding()
    {
        endingStep = 0;

        combat.ClearInput();

        currentState =
            GameState::Ending;
    }


    void FinishFinalBoss()
    {
        if (
            !ContainsId(
                saveData.recoveredWordIds,
                FINAL_WORD_ID
            )
        )
        {
            saveData
                .recoveredWordIds
                .push_back(
                    FINAL_WORD_ID
                );


            saveData.wordsRecovered =
                static_cast<int>(
                    saveData
                    .recoveredWordIds
                    .size()
                );
        }


        SaveProgress();


        if (storyReader)
        {
            storyReader
                ->SetRecoveredWords(
                    saveData.recoveredWordIds
                );
        }


        BeginEnding();
    }


    void HandleFinalBossIntroInput()
    {
        if (
            !IsKeyPressed(KEY_ENTER)
            &&
            !IsKeyPressed(KEY_SPACE)
        )
        {
            return;
        }


        finalBossIntroStep++;


        const int finalIntroStep =
            2
            +
            finalBoss.GetTheftLineCount();


        if (
            finalBossIntroStep
            >
            finalIntroStep
        )
        {
            combat.ClearInput();

            currentState =
                GameState::FinalBoss;
        }
    }


    void DrawFinalBossIntro()
    {
        ClearBackground(
            Color{
                5,
                5,
                10,
                255
            }
        );


        if (finalBossIntroStep == 0)
        {
            const char* line =
                "The final page turns.";


            DrawGameText(
                line,
                GAME_WIDTH / 2
                -
                MeasureGameText(
                    line,
                    30
                )
                /
                2,
                260,
                30,
                Color{
                    215,
                    215,
                    220,
                    255
                }
            );
        }


        else if (finalBossIntroStep == 1)
        {
            const char* line =
                "Something is waiting.";


            DrawGameText(
                line,
                GAME_WIDTH / 2
                -
                MeasureGameText(
                    line,
                    30
                )
                /
                2,
                260,
                30,
                Color{
                    215,
                    215,
                    220,
                    255
                }
            );
        }


        else
        {
            DrawGameText(
                "THE STAINED AUTHOR",
                GAME_WIDTH / 2
                -
                MeasureGameText(
                    "THE STAINED AUTHOR",
                    46
                )
                /
                2,
                90,
                46,
                Color{
                    230,
                    205,
                    145,
                    255
                }
            );


            DrawPlayer();


            const int theftIndex =
                finalBossIntroStep
                -
                2;


            if (
                theftIndex >= 0
                &&
                theftIndex
                <
                finalBoss.GetTheftLineCount()
            )
            {
                const char* line =
                    finalBoss.GetTheftLine(
                        theftIndex
                    );


                DrawGameText(
                    line,
                    GAME_WIDTH / 2
                    -
                    MeasureGameText(
                        line,
                        26
                    )
                    /
                    2,
                    270,
                    26,
                    theftIndex
                    ==
                    finalBoss.GetTheftLineCount()
                    -
                    1
                    ?
                    Color{
                        235,
                        190,
                        115,
                        255
                    }
                    :
                    Color{
                        220,
                        220,
                        220,
                        255
                    }
                );
            }
        }


        const int finalIntroStep =
            2
            +
            finalBoss.GetTheftLineCount()
            -
            1;


        const char* prompt =
            finalBossIntroStep
            >=
            finalIntroStep
            ?
            "ENTER - Face the Author"
            :
            "ENTER - Continue";


        DrawGameText(
            prompt,
            GAME_WIDTH / 2
            -
            MeasureGameText(
                prompt,
                20
            )
            /
            2,
            GAME_HEIGHT - 55,
            20,
            Color{
                150,
                145,
                155,
                255
            }
        );
    }


    void UpdateFinalBoss()
    {
        if (finalBossGameOver)
        {
            if (
                IsKeyPressed(KEY_W)
                ||
                IsKeyPressed(KEY_UP)
                ||
                IsKeyPressed(KEY_S)
                ||
                IsKeyPressed(KEY_DOWN)
            )
            {
                finalBossGameOverSelected =
                    (
                        finalBossGameOverSelected
                        +
                        1
                    )
                    %
                    2;
            }


            if (IsKeyPressed(KEY_ENTER))
            {
                if (
                    finalBossGameOverSelected
                    ==
                    0
                )
                {
                    playerHealth =
                        MAX_HEALTH;


                    combat.ClearInput();


                    finalBoss.Reset();


                    finalBossGameOver =
                        false;


                    finalBossGameOverSelected =
                        0;
                }

                else
                {
                    ReturnToMainMenu();
                }
            }


            return;
        }


        const bool playerHit =
            finalBoss.Update();


        if (playerHit)
        {
            playerHealth--;


            combat.ClearInput();


            if (playerHealth <= 0)
            {
                playerHealth =
                    0;


                finalBossGameOver =
                    true;


                finalBossGameOverSelected =
                    0;


                return;
            }
        }


        if (
            finalBoss.IsReacting()
            ||
            finalBoss.IsSlashing()
        )
        {
            combat.ClearInput();

            return;
        }


        combat.HandleInput();


        const std::string typed =
            NormalizeTypingText(
                combat.GetInput()
            );


        const std::string target =
            NormalizeTypingText(
                finalBoss.GetCurrentWord()
            );


        if (
            !typed.empty()
            &&
            typed == target
        )
        {
            combat.ClearInput();


            const bool finished =
                finalBoss
                    .CompleteCurrentWord();


            if (finished)
            {
                FinishFinalBoss();
            }
        }
    }


    void DrawFinalBoss()
    {
        ClearBackground(
            Color{
                7,
                6,
                12,
                255
            }
        );


        // Keep the Word Seeker visible during the final battle.
        // The Stained Author remains centered as the focal point.
        DrawPlayer();


        // The Word Seeker still has three hearts in the final fight.
        for (
            int index = 0;
            index < MAX_HEALTH;
            index++
        )
        {
            DrawHeart(
                35
                +
                index * 35,
                30,
                index
                <
                playerHealth
            );
        }


        finalBoss.Draw();


        if (finalBossGameOver)
        {
            DrawRectangle(
                0,
                0,
                GAME_WIDTH,
                GAME_HEIGHT,
                Color{
                    0,
                    0,
                    0,
                    215
                }
            );


            const char* title =
                "The story has come to a close, for now.";


            DrawGameText(
                title,
                GAME_WIDTH / 2
                -
                MeasureGameText(
                    title,
                    36
                )
                /
                2,
                210,
                36,
                Color{
                    235,
                    220,
                    175,
                    255
                }
            );


            const char* options[] =
            {
                "Face The Stained Author Again",
                "Main Menu"
            };


            for (
                int index = 0;
                index < 2;
                index++
            )
            {
                const Color color =
                    index
                    ==
                    finalBossGameOverSelected
                    ?
                    Color{
                        255,
                        225,
                        90,
                        255
                    }
                    :
                    Color{
                        235,
                        235,
                        235,
                        255
                    };


                DrawGameText(
                    options[index],
                    GAME_WIDTH / 2
                    -
                    MeasureGameText(
                        options[index],
                        28
                    )
                    /
                    2,
                    290
                    +
                    index * 48,
                    28,
                    color
                );
            }


            return;
        }


        if (finalBoss.IsReacting())
        {
            return;
        }


        if (finalBoss.IsSlashing())
        {
            return;
        }


        std::string target =
            finalBoss.GetCurrentWord();


        std::string typed =
            combat.GetInput();


        for (char& character : target)
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


        for (char& character : typed)
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


        bool correctPrefix =
            typed.size()
            <=
            target.size();


        if (correctPrefix)
        {
            correctPrefix =
                target.compare(
                    0,
                    typed.size(),
                    typed
                )
                ==
                0;
        }


        int highlightedCount =
            0;


        if (correctPrefix)
        {
            highlightedCount =
                static_cast<int>(
                    typed.size()
                );
        }


        const std::string completed =
            target.substr(
                0,
                highlightedCount
            );


        const std::string remaining =
            target.substr(
                highlightedCount
            );


        const int fontSize =
            34;


        const int totalWidth =
            MeasureGameText(
                target,
                fontSize
            );


        const int startX =
            GAME_WIDTH / 2
            -
            totalWidth / 2;


        // Correctly typed letters turn gold.
        DrawGameText(
            completed,
            startX,
            GAME_HEIGHT - 83,
            fontSize,
            Color{
                255,
                215,
                80,
                255
            }
        );


        // The rest of the target remains white.
        // If the player mistypes, the entire target stays white.
        DrawGameText(
            remaining,
            startX
            +
            MeasureGameText(
                completed,
                fontSize
            ),
            GAME_HEIGHT - 83,
            fontSize,
            Color{
                220,
                220,
                220,
                255
            }
        );


        // Give immediate feedback for an incorrect input
        // without pretending those letters damaged the boss.
        if (
            !typed.empty()
            &&
            !correctPrefix
        )
        {
            DrawGameText(
                typed,
                playerX
                -
                MeasureGameText(
                    typed,
                    22
                )
                /
                2,
                playerY + 55,
                22,
                Color{
                    210,
                    90,
                    90,
                    255
                }
            );
        }
    }


    void HandleEndingInput()
    {
        if (
            !IsKeyPressed(KEY_ENTER)
            &&
            !IsKeyPressed(KEY_SPACE)
        )
        {
            return;
        }


        endingStep++;


        if (endingStep >= 6)
        {
            // The boss has been faced and the final
            // exchange has happened. SAFE is now earned.
            saveData.memoryStage =
                std::max(
                    saveData.memoryStage,
                    10
                );


            SaveProgress();


            memorySystem.BeginFragment(
                9
            );


            memoryReturnsToAdventure =
                false;


            memoryShouldAdvanceWave =
                false;


            currentState =
                GameState::Memory;
        }
    }


    void DrawEnding()
    {
        ClearBackground(
            Color{
                8,
                8,
                14,
                255
            }
        );


        const char* lines[] =
        {
            "The Stained Author stops.",
            "\"That was all I was ever supposed to be.\"",
            "No.",
            "\"That was where you were supposed to be.\"",
            "\"Not what you were supposed to be.\"",
            "I remembered your place."
        };


        DrawGameText(
            "THE END",
            GAME_WIDTH / 2
            -
            MeasureGameText(
                "THE END",
                42
            )
            /
            2,
            70,
            42,
            Color{
                225,
                210,
                165,
                255
            }
        );


        for (
            int index = 0;
            index <= endingStep
            &&
            index < 6;
            index++
        )
        {
            DrawGameText(
                lines[index],
                GAME_WIDTH / 2
                -
                MeasureGameText(
                    lines[index],
                    25
                )
                /
                2,
                165 + index * 45,
                25,
                Color{
                    225,
                    225,
                    220,
                    255
                }
            );
        }


        DrawGameText(
            endingStep >= 5
            ?
            "ENTER - Remember"
            :
            "ENTER - Continue",
            GAME_WIDTH / 2
            -
            MeasureGameText(
                endingStep >= 5
                ?
                "ENTER - Remember"
                :
                "ENTER - Continue",
                19
            )
            /
            2,
            GAME_HEIGHT - 48,
            19,
            Color{
                155,
                150,
                160,
                255
            }
        );
    }


    void NewGame()
    {
        saveData
            .recoveredWordIds
            .clear();


        saveData.wordsRecovered = 0;

        saveData.memoryStage = 0;


        quillSystem.Reset();

        quillSystem.SyncUnlocks(
            0
        );


        pendingChapterUnlock =
            -1;

        chapterUnlockNumber =
            -1;

        wrongInputFrame =
            0;


        playerHealth =
            MAX_HEALTH;


        healProgress = 0;

        waveNumber = 1;

        paused = false;

        pauseSelected = 0;

        gameOver = false;

        gameOverSelected = 0;


        reservedWordIds.clear();

        enemies.clear();

        combat.ClearInput();


        SaveProgress();


        if (storyReader)
        {
            storyReader
                ->SetRecoveredWords(
                    saveData
                    .recoveredWordIds
                );
        }


        StartWave();


        memorySystem.BeginPrologue();

        memoryReturnsToAdventure = true;

        memoryShouldAdvanceWave = false;

        currentState =
            GameState::Memory;
    }


    void RestartWave()
    {
        playerHealth =
            MAX_HEALTH;


        healProgress = 0;

        paused = false;

        pauseSelected = 0;

        gameOver = false;

        gameOverSelected = 0;


        reservedWordIds.clear();

        enemies.clear();

        combat.ClearInput();


        StartWave();
    }


    void ReturnToMainMenu()
    {
        reservedWordIds.clear();

        enemies.clear();

        wordSparkEffects.clear();

        pageRestorePulse = 0;

        pendingChapterUnlock = -1;

        chapterUnlockNumber = -1;

        wrongInputFrame = 0;

        ResetWaveQueueState();

        combat.ClearInput();

        paused = false;

        pauseSelected = 0;

        gameOver = false;

        gameOverSelected = 0;

        currentState =
            GameState::Menu;
    }


    void UpdateAdventure()
    {
        if (
            paused
            ||
            gameOver
        )
        {
            return;
        }


        quillSystem.Update(
            true
        );


        UpdateWordSparks();


        if (pageRestorePulse > 0)
        {
            pageRestorePulse--;
        }


        if (CurrentInputIsWrong())
        {
            wrongInputFrame++;
        }

        else
        {
            wrongInputFrame = 0;
        }


        for (Enemy& enemy : enemies)
        {
            if (enemy.IsDefeated())
            {
                enemy.UpdateDefeat();

                continue;
            }


            if (enemy.HasEscaped())
            {
                continue;
            }


            enemy.Update();


            if (enemy.HasReachedTarget())
            {
                EnemyReachedPlayer(
                    enemy
                );


                if (gameOver)
                {
                    return;
                }
            }
        }


        enemies.erase(
            std::remove_if(
                enemies.begin(),
                enemies.end(),
                [](
                    const Enemy& enemy
                )
                {
                    return
                        enemy.HasEscaped()
                        ||
                        enemy
                        .IsDefeatFinished();
                }
            ),
            enemies.end()
        );


        UpdateQueuedSpawns();


        // The wave is not clear until both the active
        // enemies and the waiting queue are empty.
        if (
            !enemies.empty()
            ||
            queuedEnemies > 0
        )
        {
            return;
        }


        if (
            saveData.wordsRecovered
            >=
            wordManager
            .GetTotalWordCount()
        )
        {
            return;
        }


        // Once every normal word has returned, the final
        // missing word can only come from The Stained Author.
        if (
            saveData.wordsRecovered
            >=
            FINAL_NORMAL_WORD_COUNT
            &&
            !ContainsId(
                saveData.recoveredWordIds,
                FINAL_WORD_ID
            )
        )
        {
            StartFinalBoss();

            return;
        }


        if (waveClearTimer <= 0)
        {
            waveClearTimer = 90;
        }

        else
        {
            waveClearTimer--;


            if (waveClearTimer <= 0)
            {
                if (pendingChapterUnlock > 0)
                {
                    chapterUnlockNumber =
                        pendingChapterUnlock;


                    pendingChapterUnlock =
                        -1;


                    chapterUnlockPhase =
                        ChapterUnlockPhase::Closing;


                    chapterUnlockFrame =
                        0;


                    combat.ClearInput();


                    currentState =
                        GameState::ChapterUnlock;


                    return;
                }


                int memoryIndex =
                    memorySystem.GetNextMemoryIndex(
                        saveData.wordsRecovered,
                        saveData.memoryStage
                    );


                // Memory X: SAFE is reserved for after
                // The Stained Author has been faced.
                if (memoryIndex == 9)
                {
                    memoryIndex = -1;
                }


                if (memoryIndex >= 0)
                {
                    memorySystem.BeginFragment(
                        memoryIndex
                    );


                    saveData.memoryStage =
                        memoryIndex + 1;


                    SaveProgress();


                    memoryReturnsToAdventure =
                        true;


                    memoryShouldAdvanceWave =
                        true;


                    currentState =
                        GameState::Memory;


                    return;
                }


                waveNumber++;

                StartWave();
            }
        }
    }


    void ProcessCombatMatch()
    {
        if (
            paused
            ||
            gameOver
        )
        {
            return;
        }


        Enemy* enemy =
            combat.CheckAttack(
                enemies
            );


        if (enemy == nullptr)
        {
            return;
        }


        const int currentWordId =
            enemy
            ->GetCurrentWordId();


        const int enemyLane =
            FindLaneForWord(
                currentWordId
            );


        LaunchWordSpark(
            *enemy
        );


        if (currentWordId >= 0)
        {
            RecoverWord(
                currentWordId
            );
        }


        const bool enemyFinished =
            enemy
            ->CompleteCurrentWord();


        if (enemyFinished)
        {
            enemy->Defeat();


            ReleaseLane(
                enemyLane
            );
        }
    }


    std::string NormalizeTypingText(
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
                        character
                        -
                        'A'
                        +
                        'a'
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


    float EnemySpriteDrawSize(
        EnemyType type
    )
    {
        switch (type)
        {
            case EnemyType::Goblin:
                return 54.0f;

            case EnemyType::Orc:
                return 68.0f;

            case EnemyType::Wolf:
            case EnemyType::Beast:
                return 64.0f;

            case EnemyType::Bat:
                return 60.0f;

            case EnemyType::Dragon:
                return 78.0f;

            default:
                return 62.0f;
        }
    }


    void DrawOutlinedStar(
        float centerX,
        float centerY
    )
    {
        constexpr float PI_VALUE =
            3.14159265358979323846f;


        auto drawStar =
            [centerX, centerY, PI_VALUE](
                float radius,
                Color color
            )
            {
                Vector2 points[10];


                for (
                    int index = 0;
                    index < 10;
                    index++
                )
                {
                    const float angle =
                        -PI_VALUE / 2.0f
                        +
                        index * PI_VALUE / 5.0f;


                    const float pointRadius =
                        index % 2 == 0
                        ?
                        radius
                        :
                        radius * 0.45f;


                    points[index] =
                    {
                        centerX
                        +
                        std::cos(angle)
                        *
                        pointRadius,

                        centerY
                        +
                        std::sin(angle)
                        *
                        pointRadius
                    };
                }


                DrawTriangleFan(
                    points,
                    10,
                    color
                );
            };


        drawStar(
            8.0f,
            Color{
                35,
                32,
                30,
                235
            }
        );


        drawStar(
            6.0f,
            Color{
                245,
                215,
                70,
                255
            }
        );
    }


    bool CurrentInputIsWrong()
    {
        const std::string typed =
            NormalizeTypingText(
                combat.GetInput()
            );


        if (typed.empty())
        {
            return false;
        }


        if (
            !quillSystem
                .GetMatchingCommand(
                    combat.GetInput()
                )
                .empty()
        )
        {
            return false;
        }


        for (
            const Enemy& enemy
            :
            enemies
        )
        {
            if (
                enemy.IsDefeated()
                ||
                enemy.HasEscaped()
            )
            {
                continue;
            }


            const std::string target =
                NormalizeTypingText(
                    enemy.GetWordDisplay()
                );


            if (
                !target.empty()
                &&
                typed.size() <= target.size()
                &&
                target.compare(
                    0,
                    typed.size(),
                    typed
                )
                ==
                0
            )
            {
                return false;
            }
        }


        return true;
    }


    void DrawChapterUnlockCover()
    {
        DrawRectangle(
            0,
            0,
            GAME_WIDTH,
            GAME_HEIGHT,
            Color{
                20,
                12,
                18,
                245
            }
        );


        const Rectangle shadow =
        {
            48.0f,
            34.0f,
            520.0f,
            530.0f
        };


        DrawRectangleRounded(
            shadow,
            0.018f,
            8,
            Color{
                0,
                0,
                0,
                150
            }
        );


        const Rectangle cover =
        {
            31.0f,
            25.0f,
            520.0f,
            530.0f
        };


        DrawRectangleRounded(
            cover,
            0.018f,
            8,
            Color{
                72,
                31,
                82,
                255
            }
        );


        DrawRectangle(
            31,
            25,
            22,
            530,
            Color{
                55,
                23,
                67,
                255
            }
        );


        DrawRectangleRoundedLines(
            Rectangle{
                57.0f,
                47.0f,
                468.0f,
                482.0f
            },
            0.018f,
            8,
            Color{
                190,
                154,
                49,
                255
            }
        );


        DrawRectangleRoundedLines(
            Rectangle{
                66.0f,
                56.0f,
                450.0f,
                464.0f
            },
            0.018f,
            8,
            Color{
                190,
                154,
                49,
                210
            }
        );


        auto centered =
            [](
                const std::string& text,
                int y,
                int fontSize,
                Color color
            )
            {
                DrawGameText(
                    text,
                    291
                    -
                    MeasureGameText(
                        text,
                        fontSize
                    )
                    /
                    2,
                    y,
                    fontSize,
                    color
                );
            };


        const Color gold =
        {
            225,
            199,
            100,
            255
        };


        centered(
            "Fantasy Library",
            78,
            46,
            gold
        );


        centered(
            "Quest of the Word Seeker",
            150,
            27,
            gold
        );


        centered(
            "Bearly: A Sticky Adventure",
            205,
            24,
            gold
        );


        centered(
            "NEW CHAPTER UNLOCKED",
            292,
            31,
            Color{
                255,
                222,
                105,
                255
            }
        );


        centered(
            "You can read the new chapter",
            354,
            21,
            Color{
                235,
                220,
                180,
                255
            }
        );


        centered(
            "from the Main Menu.",
            382,
            21,
            Color{
                235,
                220,
                180,
                255
            }
        );


        centered(
            "ENTER - Continue",
            472,
            20,
            gold
        );
    }


    void DrawBookClosingTransition()
    {
        if (
            chapterUnlockPhase
            ==
            ChapterUnlockPhase::Showing
        )
        {
            DrawChapterUnlockCover();

            return;
        }


        const int duration =
            chapterUnlockPhase
            ==
            ChapterUnlockPhase::Closing
            ?
            CHAPTER_CLOSE_FRAMES
            :
            CHAPTER_OPEN_FRAMES;


        float progress =
            std::clamp(
                static_cast<float>(
                    chapterUnlockFrame
                )
                /
                static_cast<float>(
                    duration
                ),
                0.0f,
                1.0f
            );


        if (
            chapterUnlockPhase
            ==
            ChapterUnlockPhase::Opening
        )
        {
            progress =
                1.0f
                -
                progress;
        }


        // Reveal the purple cover beneath the closing left page.
        const float coverReveal =
            std::clamp(
                (
                    progress
                    -
                    0.25f
                )
                /
                0.75f,
                0.0f,
                1.0f
            );


        if (coverReveal > 0.0f)
        {
            DrawChapterUnlockCover();


            DrawRectangle(
                0,
                0,
                static_cast<int>(
                    GAME_WIDTH
                    *
                    (
                        1.0f
                        -
                        coverReveal
                    )
                ),
                GAME_HEIGHT,
                Color{
                    0,
                    0,
                    0,
                    static_cast<unsigned char>(
                        190
                        *
                        (
                            1.0f
                            -
                            coverReveal
                        )
                    )
                }
            );
        }


        const float foldX =
            GAME_WIDTH
            *
            0.5f
            *
            (
                1.0f
                +
                progress
            );


        const float leftEdge =
            GAME_WIDTH
            *
            progress;


        DrawTriangle(
            Vector2{
                leftEdge,
                0.0f
            },
            Vector2{
                foldX,
                static_cast<float>(
                    GAME_HEIGHT
                )
            },
            Vector2{
                foldX,
                0.0f
            },
            Color{
                205,
                179,
                122,
                255
            }
        );


        DrawTriangle(
            Vector2{
                leftEdge,
                0.0f
            },
            Vector2{
                leftEdge,
                static_cast<float>(
                    GAME_HEIGHT
                )
            },
            Vector2{
                foldX,
                static_cast<float>(
                    GAME_HEIGHT
                )
            },
            Color{
                177,
                145,
                96,
                255
            }
        );


        DrawLineEx(
            Vector2{
                foldX,
                0.0f
            },
            Vector2{
                foldX,
                static_cast<float>(
                    GAME_HEIGHT
                )
            },
            4.0f,
            Color{
                245,
                220,
                166,
                220
            }
        );
    }


    void UpdateChapterUnlock()
    {
        if (
            chapterUnlockPhase
            ==
            ChapterUnlockPhase::Showing
        )
        {
            return;
        }


        chapterUnlockFrame++;


        const int duration =
            chapterUnlockPhase
            ==
            ChapterUnlockPhase::Closing
            ?
            CHAPTER_CLOSE_FRAMES
            :
            CHAPTER_OPEN_FRAMES;


        if (chapterUnlockFrame < duration)
        {
            return;
        }


        chapterUnlockFrame = 0;


        if (
            chapterUnlockPhase
            ==
            ChapterUnlockPhase::Closing
        )
        {
            chapterUnlockPhase =
                ChapterUnlockPhase::Showing;
        }

        else
        {
            chapterUnlockNumber =
                -1;


            waveNumber++;

            StartWave();


            currentState =
                GameState::Adventure;
        }
    }


    void DrawEnemyTypingHighlight(
        const Enemy& enemy
    )
    {
        if (
            enemy.IsDefeated()
            ||
            enemy.HasEscaped()
        )
        {
            return;
        }


        std::string fullUpper =
            enemy.GetWordDisplay();


        if (fullUpper.empty())
        {
            return;
        }


        for (
            char& character
            :
            fullUpper
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


        constexpr int fontSize = 27;
        constexpr int paddingX = 12;
        constexpr int paddingY = 7;


        const bool wrongInput =
            CurrentInputIsWrong();


        const int shake =
            wrongInput
            ?
            (
                (
                    wrongInputFrame / 2
                )
                %
                2
                ==
                0
                ?
                -4
                :
                4
            )
            :
            0;


        const int fullWidth =
            MeasureGameText(
                fullUpper,
                fontSize
            );


        const float labelOffset =
            enemy.GetEnemyType()
            ==
            EnemyType::Dragon
            ?
            76.0f
            :
            64.0f;


        const int textX =
            static_cast<int>(
                enemy.GetX()
            )
            -
            fullWidth / 2
            +
            shake;


        const int textY =
            static_cast<int>(
                enemy.GetY()
                -
                labelOffset
            );


        const Rectangle bubble =
        {
            static_cast<float>(
                textX
                -
                paddingX
            ),
            static_cast<float>(
                textY
                -
                paddingY
            ),
            static_cast<float>(
                fullWidth
                +
                paddingX * 2
            ),
            static_cast<float>(
                fontSize
                +
                paddingY * 2
            )
        };


        // Tiny shadow.
        DrawRectangleRounded(
            Rectangle{
                bubble.x + 3.0f,
                bubble.y + 3.0f,
                bubble.width,
                bubble.height
            },
            0.22f,
            8,
            Color{
                0,
                0,
                0,
                115
            }
        );


        // Hanging bookmark/speech line tying the word to its creature.
        const Vector2 lineStart =
        {
            bubble.x
            +
            bubble.width
            *
            0.68f,
            bubble.y
            +
            bubble.height
        };


        const Vector2 lineEnd =
        {
            enemy.GetX()
            -
            EnemySpriteDrawSize(
                enemy.GetEnemyType()
            )
            *
            0.18f,
            enemy.GetY()
            -
            EnemySpriteDrawSize(
                enemy.GetEnemyType()
            )
            *
            0.30f
        };


        DrawLineEx(
            Vector2{
                lineStart.x + 2.0f,
                lineStart.y + 2.0f
            },
            Vector2{
                lineEnd.x + 2.0f,
                lineEnd.y + 2.0f
            },
            3.0f,
            Color{
                0,
                0,
                0,
                95
            }
        );


        DrawLineEx(
            lineStart,
            lineEnd,
            2.0f,
            Color{
                111,
                85,
                54,
                230
            }
        );


        DrawRectangleRounded(
            bubble,
            0.22f,
            8,
            Color{
                18,
                18,
                22,
                218
            }
        );


        DrawRectangleRoundedLines(
            bubble,
            0.22f,
            8,
            Color{
                111,
                85,
                54,
                255
            }
        );


        const int stars =
            enemy.GetStarCount();


        if (stars > 0)
        {
            const float starX =
                bubble.x
                +
                bubble.width
                +
                13.0f;


            const float firstStarY =
                bubble.y
                +
                bubble.height / 2.0f
                -
                (
                    stars - 1
                )
                *
                9.0f;


            for (
                int index = 0;
                index < stars;
                index++
            )
            {
                DrawOutlinedStar(
                    starX,
                    firstStarY
                    +
                    index * 18.0f
                );
            }
        }


        DrawGameText(
            fullUpper,
            textX,
            textY,
            fontSize,
            wrongInput
            ?
            Color{
                235,
                82,
                82,
                255
            }
            :
            Color{
                232,
                232,
                235,
                255
            }
        );


        if (wrongInput)
        {
            return;
        }


        const std::string typed =
            NormalizeTypingText(
                combat.GetInput()
            );


        if (
            typed.empty()
            ||
            !quillSystem
                .GetMatchingCommand(
                    combat.GetInput()
                )
                .empty()
        )
        {
            return;
        }


        const std::string target =
            NormalizeTypingText(
                fullUpper
            );


        if (
            typed.size() > target.size()
            ||
            target.compare(
                0,
                typed.size(),
                typed
            )
            !=
            0
        )
        {
            return;
        }


        std::size_t visibleCharacters = 0;
        std::size_t endIndex = 0;


        while (
            endIndex < fullUpper.size()
            &&
            visibleCharacters < typed.size()
        )
        {
            const char character =
                fullUpper[endIndex];


            if (
                character >= 'A'
                &&
                character <= 'Z'
            )
            {
                visibleCharacters++;
            }


            endIndex++;
        }


        DrawGameText(
            fullUpper.substr(
                0,
                endIndex
            ),
            textX,
            textY,
            fontSize,
            Color{
                255,
                215,
                70,
                255
            }
        );
    }

    float GetWorldRestorationProgress()
    {
        return std::clamp(
            static_cast<float>(
                saveData.wordsRecovered
            )
            /
            static_cast<float>(
                FINAL_NORMAL_WORD_COUNT
            ),
            0.0f,
            1.0f
        );
    }


    void DrawParchmentPage()
    {
        const int width =
            GAME_WIDTH;


        const int height =
            GAME_HEIGHT;


        // Base antique parchment.
        ClearBackground(
            Color{
                174,
                145,
                94,
                255
            }
        );


        const int margin =
            std::max(
                18,
                width / 42
            );


        // Slightly lighter center field.
        DrawRectangle(
            margin,
            margin,
            width - margin * 2,
            height - margin * 2,
            Color{
                194,
                165,
                109,
                255
            }
        );


        // Aged page edges.
        for (
            int layer = 0;
            layer < 6;
            layer++
        )
        {
            const int inset =
                layer * 5;


            DrawRectangleLinesEx(
                Rectangle{
                    static_cast<float>(
                        inset
                    ),
                    static_cast<float>(
                        inset
                    ),
                    static_cast<float>(
                        width
                        -
                        inset * 2
                    ),
                    static_cast<float>(
                        height
                        -
                        inset * 2
                    )
                },
                5.0f,
                Color{
                    76,
                    52,
                    29,
                    static_cast<unsigned char>(
                        100
                        -
                        layer * 12
                    )
                }
            );
        }


        // Faint manuscript ruling.
        const int lineSpacing =
            std::max(
                32,
                height / 15
            );


        for (
            int y =
                margin
                +
                lineSpacing;
            y
            <
                height
                -
                margin;
            y +=
                lineSpacing
        )
        {
            DrawLine(
                margin + 18,
                y,
                width - margin - 18,
                y,
                Color{
                    103,
                    74,
                    43,
                    38
                }
            );
        }


        // Very subtle book/page crease.
        DrawRectangle(
            width / 2 - 2,
            margin,
            4,
            height - margin * 2,
            Color{
                91,
                61,
                35,
                24
            }
        );


        // Deterministic fibers and age marks.
        for (
            int index = 0;
            index < 34;
            index++
        )
        {
            const int x =
                (
                    index * 137
                    +
                    53
                )
                %
                std::max(
                    1,
                    width
                );


            const int y =
                (
                    index * 83
                    +
                    29
                )
                %
                std::max(
                    1,
                    height
                );


            const int radius =
                1
                +
                index % 3;


            DrawCircle(
                x,
                y,
                static_cast<float>(
                    radius
                ),
                Color{
                    105,
                    74,
                    42,
                    22
                }
            );
        }
    }


    void DrawRestoringPageBackground()
    {
        const int width =
            GAME_WIDTH;


        const int height =
            GAME_HEIGHT;


        const float progress =
            GetWorldRestorationProgress();


        // Draw the fully restored page first. Darkness is then
        // layered over it everywhere except the expanding safe
        // region around the Word Seeker.
        DrawParchmentPage();


        // At zero words there is still a small clear circle around
        // the player so it immediately reads as "the world exists
        // beneath the stain."
        const float minimumRadius =
            std::max(
                72.0f,
                static_cast<float>(
                    std::min(
                        width,
                        height
                    )
                )
                *
                0.13f
            );


        // Radius needed to reach the farthest corner from player.
        const float cornerDistances[] =
        {
            std::sqrt(
                static_cast<float>(
                    playerX * playerX
                    +
                    playerY * playerY
                )
            ),

            std::sqrt(
                static_cast<float>(
                    (
                        width - playerX
                    )
                    *
                    (
                        width - playerX
                    )
                    +
                    playerY * playerY
                )
            ),

            std::sqrt(
                static_cast<float>(
                    playerX * playerX
                    +
                    (
                        height - playerY
                    )
                    *
                    (
                        height - playerY
                    )
                )
            ),

            std::sqrt(
                static_cast<float>(
                    (
                        width - playerX
                    )
                    *
                    (
                        width - playerX
                    )
                    +
                    (
                        height - playerY
                    )
                    *
                    (
                        height - playerY
                    )
                )
            )
        };


        float maximumRadius =
            cornerDistances[0];


        for (
            float distance
            :
            cornerDistances
        )
        {
            maximumRadius =
                std::max(
                    maximumRadius,
                    distance
                );
        }


        // Ease-out growth: visible progress feels rewarding early,
        // while the last stains linger near the distant edges.
        const float easedProgress =
            1.0f
            -
            (
                1.0f - progress
            )
            *
            (
                1.0f - progress
            );


        float clearRadius =
            minimumRadius
            +
            (
                maximumRadius
                -
                minimumRadius
            )
            *
            easedProgress;


        if (pageRestorePulse > 0)
        {
            const float pulseProgress =
                1.0f
                -
                static_cast<float>(
                    pageRestorePulse
                )
                /
                18.0f;


            clearRadius +=
                std::sin(
                    pulseProgress
                    *
                    PI
                )
                *
                12.0f;
        }


        // Soft transition width between parchment and deep stain.
        const float featherWidth =
            std::max(
                38.0f,
                static_cast<float>(
                    std::min(
                        width,
                        height
                    )
                )
                *
                0.075f
            );


        // Draw the stain as horizontal strips. Each strip works out
        // where the expanding circle intersects it, leaving the
        // parchment underneath visible inside the restored region.
        //
        // This avoids the old "random black bubbles" look while
        // remaining completely procedural and asset-free.
        constexpr int stripHeight =
            4;


        for (
            int y = 0;
            y < height;
            y += stripHeight
        )
        {
            const float dy =
                static_cast<float>(
                    y
                    -
                    playerY
                );


            const float absoluteDy =
                std::fabs(
                    dy
                );


            // Deep-black outer stain.
            if (absoluteDy >= clearRadius)
            {
                DrawRectangle(
                    0,
                    y,
                    width,
                    stripHeight,
                    Color{
                        4,
                        4,
                        7,
                        248
                    }
                );


                continue;
            }


            const float halfClearWidth =
                std::sqrt(
                    std::max(
                        0.0f,
                        clearRadius
                        *
                        clearRadius
                        -
                        dy
                        *
                        dy
                    )
                );


            const int clearLeft =
                static_cast<int>(
                    playerX
                    -
                    halfClearWidth
                );


            const int clearRight =
                static_cast<int>(
                    playerX
                    +
                    halfClearWidth
                );


            if (clearLeft > 0)
            {
                DrawRectangle(
                    0,
                    y,
                    clearLeft,
                    stripHeight,
                    Color{
                        4,
                        4,
                        7,
                        248
                    }
                );
            }


            if (clearRight < width)
            {
                DrawRectangle(
                    clearRight,
                    y,
                    width
                    -
                    clearRight,
                    stripHeight,
                    Color{
                        4,
                        4,
                        7,
                        248
                    }
                );
            }


            // A second, larger radius produces a soft ink boundary.
            const float softRadius =
                clearRadius
                +
                featherWidth;


            if (absoluteDy < softRadius)
            {
                const float halfSoftWidth =
                    std::sqrt(
                        std::max(
                            0.0f,
                            softRadius
                            *
                            softRadius
                            -
                            dy
                            *
                            dy
                        )
                    );


                const int softLeft =
                    static_cast<int>(
                        playerX
                        -
                        halfSoftWidth
                    );


                const int softRight =
                    static_cast<int>(
                        playerX
                        +
                        halfSoftWidth
                    );


                const int leftFadeStart =
                    std::max(
                        0,
                        softLeft
                    );


                const int leftFadeEnd =
                    std::max(
                        0,
                        clearLeft
                    );


                const int rightFadeStart =
                    std::min(
                        width,
                        clearRight
                    );


                const int rightFadeEnd =
                    std::min(
                        width,
                        softRight
                    );


                if (
                    leftFadeEnd
                    >
                    leftFadeStart
                )
                {
                    const int fadeWidth =
                        leftFadeEnd
                        -
                        leftFadeStart;


                    // Several bands make the ink edge appear soft
                    // rather than mechanically circular.
                    constexpr int bands =
                        8;


                    for (
                        int band = 0;
                        band < bands;
                        band++
                    )
                    {
                        const float t =
                            static_cast<float>(
                                band
                            )
                            /
                            static_cast<float>(
                                bands
                            );


                        const int bandX =
                            leftFadeStart
                            +
                            static_cast<int>(
                                fadeWidth * t
                            );


                        const int nextBandX =
                            leftFadeStart
                            +
                            static_cast<int>(
                                fadeWidth
                                *
                                (
                                    static_cast<float>(
                                        band + 1
                                    )
                                    /
                                    static_cast<float>(
                                        bands
                                    )
                                )
                            );


                        const unsigned char alpha =
                            static_cast<unsigned char>(
                                230.0f
                                *
                                (
                                    1.0f - t
                                )
                                *
                                (
                                    1.0f - t
                                )
                            );


                        DrawRectangle(
                            bandX,
                            y,
                            std::max(
                                1,
                                nextBandX
                                -
                                bandX
                            ),
                            stripHeight,
                            Color{
                                4,
                                4,
                                7,
                                alpha
                            }
                        );
                    }
                }


                if (
                    rightFadeEnd
                    >
                    rightFadeStart
                )
                {
                    const int fadeWidth =
                        rightFadeEnd
                        -
                        rightFadeStart;


                    constexpr int bands =
                        8;


                    for (
                        int band = 0;
                        band < bands;
                        band++
                    )
                    {
                        const float t =
                            static_cast<float>(
                                band
                            )
                            /
                            static_cast<float>(
                                bands
                            );


                        const int bandX =
                            rightFadeStart
                            +
                            static_cast<int>(
                                fadeWidth * t
                            );


                        const int nextBandX =
                            rightFadeStart
                            +
                            static_cast<int>(
                                fadeWidth
                                *
                                (
                                    static_cast<float>(
                                        band + 1
                                    )
                                    /
                                    static_cast<float>(
                                        bands
                                    )
                                )
                            );


                        const unsigned char alpha =
                            static_cast<unsigned char>(
                                230.0f
                                *
                                t
                                *
                                t
                            );


                        DrawRectangle(
                            bandX,
                            y,
                            std::max(
                                1,
                                nextBandX
                                -
                                bandX
                            ),
                            stripHeight,
                            Color{
                                4,
                                4,
                                7,
                                alpha
                            }
                        );
                    }
                }
            }
        }


        // Add a faint irregular ink edge. These tiny dark patches
        // sit only near the current restoration frontier and move
        // outward with it, so they read as a living stain rather
        // than random dots across the page.
        if (progress < 0.995f)
        {
            for (
                int index = 0;
                index < 26;
                index++
            )
            {
                const float angle =
                    (
                        static_cast<float>(
                            index
                        )
                        /
                        26.0f
                    )
                    *
                    2.0f
                    *
                    PI;


                const float wobble =
                    1.0f
                    +
                    0.035f
                    *
                    std::sin(
                        angle * 5.0f
                        +
                        static_cast<float>(
                            index % 4
                        )
                    );


                const float radius =
                    clearRadius
                    *
                    wobble;


                const int x =
                    static_cast<int>(
                        playerX
                        +
                        std::cos(
                            angle
                        )
                        *
                        radius
                    );


                const int y =
                    static_cast<int>(
                        playerY
                        +
                        std::sin(
                            angle
                        )
                        *
                        radius
                    );


                if (
                    x < -30
                    ||
                    x > width + 30
                    ||
                    y < -30
                    ||
                    y > height + 30
                )
                {
                    continue;
                }


                DrawCircle(
                    x,
                    y,
                    static_cast<float>(
                        7
                        +
                        index % 9
                    ),
                    Color{
                        5,
                        5,
                        8,
                        115
                    }
                );
            }
        }
    }


    void DrawAdventure()
    {
        DrawRestoringPageBackground();


        DrawPlayer();


        for (
            int index = 0;
            index < MAX_HEALTH;
            index++
        )
        {
            DrawHeart(
                35
                +
                index * 35,
                30,
                index
                <
                playerHealth
            );
        }


        if (
            playerHealth
            <
            MAX_HEALTH
        )
        {
            const std::string healing =
                "Healing: "
                +
                std::to_string(
                    healProgress
                )
                +
                "/10";


            DrawGameText(
                healing.c_str(),
                20,
                58,
                22,
                Color{
                    180,
                    220,
                    190,
                    255
                }
            );
        }


        const std::string progress =
            "Words Recovered: "
            +
            std::to_string(
                saveData.wordsRecovered
            )
            +
            " / "
            +
            std::to_string(
                wordManager.GetTotalWordCount()
            )
            +
            "    Creatures Remaining: "
            +
            std::to_string(
                static_cast<int>(
                    enemies.size()
                )
                +
                queuedEnemies
            );


        const int progressFontSize =
    22;


const int progressPaddingX =
    10;


const int progressPaddingY =
    6;


const int progressWidth =
    MeasureGameText(
        progress.c_str(),
        progressFontSize
    );


DrawRectangle(
    14,
    82,
    progressWidth
    +
    progressPaddingX * 2,
    progressFontSize
    +
    progressPaddingY * 2,
    Color{
        8,
        8,
        12,
        205
    }
);


DrawGameText(
    progress.c_str(),
    20,
    88,
    progressFontSize,
    Color{
        200,
        220,
        240,
        255
    }
);


        if (
            !paused
            &&
            !gameOver
        )
        {
            for (
                const Enemy& enemy
                :
                enemies
            )
            {
                enemy.Draw();

                DrawEnemyTypingHighlight(
                    enemy
                );
            }


            DrawWordSparks();


            std::string typed =
                combat.GetInput();


            for (
                char& character
                :
                typed
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


            const int textWidth =
                MeasureGameText(
                    typed.c_str(),
                    32
                );


            const bool typingQuillCommand =
                !quillSystem
                    .GetMatchingCommand(
                        combat.GetInput()
                    )
                    .empty();


            DrawGameText(
                typed.c_str(),
                playerX
                -
                textWidth / 2,
                GAME_HEIGHT
                -
                45,
                32,
                typingQuillCommand
                ?
                Color{
                    245,
                    200,
                    75,
                    255
                }
                :
                Color{
                    255,
                    230,
                    90,
                    255
                }
            );
        }


        if (
            !paused
            &&
            !gameOver
        )
        {
            quillSystem.DrawHUD(
             GAME_WIDTH,
             GAME_HEIGHT,
             playerX,
             playerY,
             true,
            combat.GetInput()
        );
    }


        if (
            waveClearTimer > 0
            &&
            !paused
            &&
            !gameOver
        )
        {
            const char* message =
                "WAVE CLEARED!";


            const int width =
                MeasureGameText(
                    message,
                    32
                );


            DrawGameText(
                message,
                GAME_WIDTH / 2
                -
                width / 2,
                115,
                32,
                Color{
                    255,
                    230,
                    120,
                    255
                }
            );
        }


        if (paused)
        {
            DrawRectangle(
                0,
                0,
                GAME_WIDTH,
                GAME_HEIGHT,
                Color{
                    0,
                    0,
                    0,
                    190
                }
            );


            const char* title =
                "PAUSED";


            const int titleWidth =
                MeasureGameText(
                    title,
                    32
                );


            DrawGameText(
                title,
                GAME_WIDTH / 2
                -
                titleWidth / 2,
                220,
                32,
                Color{
                    255,
                    230,
                    100,
                    255
                }
            );


            const char* options[] =
            {
                "Resume",
                "Main Menu"
            };


            for (
                int index = 0;
                index < 2;
                index++
            )
            {
                const Color color =
                    index
                    ==
                    pauseSelected
                    ?
                    Color{
                        255,
                        230,
                        90,
                        255
                    }
                    :
                    Color{
                        235,
                        235,
                        235,
                        255
                    };


                const int width =
                    MeasureGameText(
                        options[index],
                        32
                    );


                DrawGameText(
                    options[index],
                    GAME_WIDTH / 2
                    -
                    width / 2,
                    285
                    +
                    index * 48,
                    32,
                    color
                );
            }
        }


        if (gameOver)
        {
            DrawRectangle(
                0,
                0,
                GAME_WIDTH,
                GAME_HEIGHT,
                Color{
                    0,
                    0,
                    0,
                    210
                }
            );


            const char* title =
                "The story has come to a close, for now.";


            const int titleWidth =
                MeasureGameText(
                    title,
                    38
                );


            DrawGameText(
                title,
                GAME_WIDTH / 2
                -
                titleWidth / 2,
                215,
                38,
                Color{
                    235,
                    220,
                    175,
                    255
                }
            );


            const char* options[] =
            {
                "Restart Wave",
                "Main Menu"
            };


            for (
                int index = 0;
                index < 2;
                index++
            )
            {
                const Color color =
                    index
                    ==
                    gameOverSelected
                    ?
                    Color{
                        255,
                        230,
                        90,
                        255
                    }
                    :
                    Color{
                        235,
                        235,
                        235,
                        255
                    };


                const int width =
                    MeasureGameText(
                        options[index],
                        32
                    );


                DrawGameText(
                    options[index],
                    GAME_WIDTH / 2
                    -
                    width / 2,
                    290
                    +
                    index * 48,
                    32,
                    color
                );
            }
        }


        if (
            enemies.empty()
            &&
            saveData.wordsRecovered
            >=
            wordManager
            .GetTotalWordCount()
            &&
            !gameOver
        )
        {
            const char* restored =
                "The story has been restored!";


            const int width =
                MeasureGameText(
                    restored,
                    32
                );


            DrawGameText(
                restored,
                GAME_WIDTH / 2
                -
                width / 2,
                GAME_HEIGHT / 2,
                32,
                Color{
                    255,
                    240,
                    150,
                    255
                }
            );
        }
    }


    void HandleAdventureInput()
    {
        if (gameOver)
        {
            if (
                IsKeyPressed(KEY_W)
                ||
                IsKeyPressed(KEY_UP)
            )
            {
                gameOverSelected =
                    (
                        gameOverSelected
                        +
                        1
                    )
                    %
                    2;
            }


            else if (
                IsKeyPressed(KEY_S)
                ||
                IsKeyPressed(KEY_DOWN)
            )
            {
                gameOverSelected =
                    (
                        gameOverSelected
                        +
                        1
                    )
                    %
                    2;
            }


            else if (
                IsKeyPressed(KEY_ENTER)
            )
            {
                if (
                    gameOverSelected
                    ==
                    0
                )
                {
                    RestartWave();
                }

                else
                {
                    ReturnToMainMenu();
                }
            }


            return;
        }


        if (paused)
        {
            if (
                IsKeyPressed(KEY_ESCAPE)
            )
            {
                paused = false;

                combat.ClearInput();

                return;
            }


            if (
                IsKeyPressed(KEY_W)
                ||
                IsKeyPressed(KEY_UP)
            )
            {
                pauseSelected =
                    (
                        pauseSelected
                        +
                        1
                    )
                    %
                    2;
            }


            else if (
                IsKeyPressed(KEY_S)
                ||
                IsKeyPressed(KEY_DOWN)
            )
            {
                pauseSelected =
                    (
                        pauseSelected
                        +
                        1
                    )
                    %
                    2;
            }


            else if (
                IsKeyPressed(KEY_ENTER)
            )
            {
                if (
                    pauseSelected
                    ==
                    0
                )
                {
                    paused = false;

                    combat.ClearInput();
                }

                else
                {
                    ReturnToMainMenu();
                }
            }


            return;
        }


        if (
            IsKeyPressed(KEY_ESCAPE)
        )
        {
            paused = true;

            pauseSelected = 0;

            combat.ClearInput();

            return;
        }


        combat.HandleInput();


        const std::string completedCommand =
            quillSystem.GetCompletedCommand(
                combat.GetInput()
            );


        if (!completedCommand.empty())
        {
            if (completedCommand == "stun")
            {
                UseStunAbility();
            }

            else if (completedCommand == "freeze")
            {
                UseFreezeAbility();
            }

            else if (completedCommand == "erase")
            {
                UseEraseAbility();
            }

            else if (completedCommand == "rewrite")
            {
                UseRewriteAbility();
            }


            combat.ClearInput();

            return;
        }


        ProcessCombatMatch();
    }


    void HandleMenu()
    {
        const MenuAction action =
            menu.HandleInput();


        switch (action)
        {
            case MenuAction::Start:
            {
                currentState =
                    GameState::Adventure;


                combat.ClearInput();


                if (enemies.empty())
                {
                    StartWave();
                }


                break;
            }


            case MenuAction::Story:
            {
                if (storyReader)
                {
                    storyReader
                        ->SetRecoveredWords(
                            saveData
                            .recoveredWordIds
                        );
                }


                currentState =
                    GameState::Story;


                break;
            }


            case MenuAction::Memories:
            {
                memorySystem.ResetLibrary();

                currentState =
                    GameState::MemoryLibrary;


                break;
            }


            case MenuAction::NewGame:
            {
                NewGame();


                break;
            }


            case MenuAction::DisplayLeft:
            {
                displayIndex =
                    (
                        displayIndex
                        -
                        1
                        +
                        DISPLAY_OPTION_COUNT
                    )
                    %
                    DISPLAY_OPTION_COUNT;


                ApplyDisplaySetting();

                SaveProgress();


                break;
            }


            case MenuAction::DisplayRight:
            {
                displayIndex =
                    (
                        displayIndex
                        +
                        1
                    )
                    %
                    DISPLAY_OPTION_COUNT;


                ApplyDisplaySetting();

                SaveProgress();


                break;
            }


            case MenuAction::VolumeDown:
            {
                musicVolume =
                    std::max(
                        0,
                        musicVolume - 10
                    );


                if (backgroundMusicLoaded)
                {
                    SetMusicVolume(
                        backgroundMusic,
                        static_cast<float>(musicVolume) / 100.0f
                    );
                }


                menu.SetSettings(
                    DISPLAY_OPTIONS[
                        displayIndex
                    ].label,
                    musicVolume
                );


                SaveProgress();


                break;
            }


            case MenuAction::VolumeUp:
            {
                musicVolume =
                    std::min(
                        100,
                        musicVolume + 10
                    );


                if (backgroundMusicLoaded)
                {
                    SetMusicVolume(
                        backgroundMusic,
                        static_cast<float>(musicVolume) / 100.0f
                    );
                }


                menu.SetSettings(
                    DISPLAY_OPTIONS[
                        displayIndex
                    ].label,
                    musicVolume
                );


                SaveProgress();


                break;
            }


            case MenuAction::Exit:
            {
                running = false;

                break;
            }


            default:
                break;
        }
    }
}


int main()
{
    SetConfigFlags(
        FLAG_WINDOW_RESIZABLE
        |
        FLAG_WINDOW_HIGHDPI
    );


    InitWindow(
        GAME_WIDTH,
        GAME_HEIGHT,
        "Fantasy Library: Quest of the Word Seeker"
    );


    SetWindowMinSize(
        800,
        533
    );


    // ESC belongs to the game's pause/story menus,
    // not raylib's default window-close behavior.
    SetExitKey(
        KEY_NULL
    );


    InitGameFont();


    RenderTexture2D gameCanvas =
        LoadRenderTexture(
            GAME_WIDTH,
            GAME_HEIGHT
        );


    // Smooth the completed 900x600 frame when it is scaled to
    // non-integer display sizes such as 1200x800 or fullscreen.
    SetTextureFilter(
        gameCanvas.texture,
        TEXTURE_FILTER_BILINEAR
    );


    // Load the shared enemy sprite atlas once after the
    // raylib window/OpenGL context has been created.
    EnemySprites::Load();


    SetTargetFPS(
        FPS
    );


    saveData =
        saveManager.LoadSave();


    quillSystem.Reset();

    quillSystem.SyncUnlocks(
        saveData.wordsRecovered
    );


    displayIndex =
        std::clamp(
            saveData.displayIndex,
            0,
            DISPLAY_OPTION_COUNT - 1
        );


    musicVolume =
        std::clamp(
            saveData.musicVolume,
            0,
            100
        );


    SyncVirtualViewport();

    RefreshDisplayGeometry();

    ApplyDisplaySetting();


    storyReader =
        std::make_unique<
            StoryReader
        >(
            saveData.recoveredWordIds
        );


    SyncVirtualViewport();


    InitAudioDevice();


    if (
        IsAudioDeviceReady()
        &&
        FileExists(
            "word_seeker_storybook_loop.ogg"
        )
    )
    {
        backgroundMusic =
            LoadMusicStream(
                "word_seeker_storybook_loop.ogg"
            );


        if (backgroundMusic.ctxData != nullptr)
        {
            backgroundMusic.looping = true;

            SetMusicVolume(
                backgroundMusic,
                static_cast<float>(musicVolume) / 100.0f
            );


            PlayMusicStream(
                backgroundMusic
            );


            backgroundMusicLoaded = true;
        }
    }


    while (
        running
        &&
        !WindowShouldClose()
    )
    {
        // Window resizing affects only presentation scale.
        // The logical 900x600 gameplay canvas never changes.


        if (backgroundMusicLoaded)
        {
            UpdateMusicStream(
                backgroundMusic
            );
        }


        // -------------------------
        // INPUT / UPDATE
        // -------------------------

        if (
            currentState
            ==
            GameState::Splash
        )
        {
            UpdateSplash();
        }


        else if (
            currentState
            ==
            GameState::Menu
        )
        {
            HandleMenu();
        }


        else if (
            currentState
            ==
            GameState::Adventure
        )
        {
            HandleAdventureInput();

            UpdateAdventure();
        }


        else if (
            currentState
            ==
            GameState::Story
        )
        {
            if (
                storyReader
                &&
                storyReader
                ->HandleInput()
            )
            {
                currentState =
                    GameState::Menu;
            }
        }


        else if (
            currentState
            ==
            GameState::Memory
        )
        {
            if (
                memorySystem
                .HandleFragmentInput()
            )
            {
                if (memoryReturnsToAdventure)
                {
                    memoryReturnsToAdventure =
                        false;


                    if (memoryShouldAdvanceWave)
                    {
                        waveNumber++;

                        StartWave();
                    }


                    memoryShouldAdvanceWave =
                        false;


                    currentState =
                        GameState::Adventure;
                }

                else
                {
                    currentState =
                        GameState::Menu;
                }
            }
        }


        else if (
            currentState
            ==
            GameState::MemoryLibrary
        )
        {
            if (
                memorySystem
                .HandleLibraryInput(
                    saveData.memoryStage
                )
            )
            {
                currentState =
                    GameState::Menu;
            }
        }


        else if (
            currentState
            ==
            GameState::ChapterUnlock
        )
        {
            if (
                chapterUnlockPhase
                ==
                ChapterUnlockPhase::Showing
                &&
                (
                    IsKeyPressed(KEY_ENTER)
                    ||
                    IsKeyPressed(KEY_SPACE)
                )
            )
            {
                chapterUnlockPhase =
                    ChapterUnlockPhase::Opening;


                chapterUnlockFrame =
                    0;
            }


            UpdateChapterUnlock();
        }


        else if (
            currentState
            ==
            GameState::FinalBossIntro
        )
        {
            HandleFinalBossIntroInput();
        }


        else if (
            currentState
            ==
            GameState::FinalBoss
        )
        {
            UpdateFinalBoss();
        }


        else if (
            currentState
            ==
            GameState::Ending
        )
        {
            HandleEndingInput();
        }


        // -------------------------
        // DRAW
        // -------------------------

        BeginTextureMode(
            gameCanvas
        );


        if (
            currentState
            ==
            GameState::Splash
        )
        {
            DrawSplash();
        }


        else if (
            currentState
            ==
            GameState::Menu
        )
        {
            menu.Draw();
        }


        else if (
            currentState
            ==
            GameState::Adventure
        )
        {
            DrawAdventure();
        }


        else if (
            currentState
            ==
            GameState::Story
        )
        {
            if (storyReader)
            {
                storyReader->Draw();
            }
        }


        else if (
            currentState
            ==
            GameState::Memory
        )
        {
            memorySystem.DrawFragment();
        }


        else if (
            currentState
            ==
            GameState::MemoryLibrary
        )
        {
            memorySystem.DrawLibrary(
                saveData.memoryStage
            );
        }


        else if (
            currentState
            ==
            GameState::ChapterUnlock
        )
        {
            DrawAdventure();

            DrawBookClosingTransition();
        }


        else if (
            currentState
            ==
            GameState::FinalBossIntro
        )
        {
            DrawFinalBossIntro();
        }


        else if (
            currentState
            ==
            GameState::FinalBoss
        )
        {
            DrawFinalBoss();
        }


        else if (
            currentState
            ==
            GameState::Ending
        )
        {
            DrawEnding();
        }


        EndTextureMode();


        BeginDrawing();

        DrawVirtualCanvasToWindow(
            gameCanvas
        );

        EndDrawing();
    }


    if (backgroundMusicLoaded)
    {
        StopMusicStream(
            backgroundMusic
        );


        UnloadMusicStream(
            backgroundMusic
        );
    }


    if (IsAudioDeviceReady())
    {
        CloseAudioDevice();
    }


    SaveProgress();


    storyReader.reset();


    EnemySprites::Unload();


    UnloadRenderTexture(
        gameCanvas
    );


    ShutdownGameFont();


    CloseWindow();


    return 0;
}