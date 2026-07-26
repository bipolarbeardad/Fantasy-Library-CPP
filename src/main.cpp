#include "raylib.h"

#include "Combat.h"
#include "Enemy.h"
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

    // ID 1125 is reserved exclusively for The Stained Author.
    constexpr int FINAL_WORD_ID = 1125;
    constexpr int FINAL_NORMAL_WORD_COUNT = 1209;

    enum class GameState
    {
        Menu,
        Adventure,
        Story,
        Memory,
        MemoryLibrary,
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
        GameState::Menu;


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


    int chapterRestoredTimer = 0;

    int chapterRestoredNumber = -1;


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


    // Forward declaration because the final boss retry/menu
    // handler uses this before its full definition later.
    void ReturnToMainMenu();


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
        const int width =
            GetScreenWidth();


        const int height =
            GetScreenHeight();


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
                GetScreenWidth()
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
            chapterRestoredNumber =
                chapterBeforeRecovery;


            chapterRestoredTimer =
                180;
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
                GetScreenWidth() / 2
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
                GetScreenWidth() / 2
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
                GetScreenWidth() / 2
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
                    GetScreenWidth() / 2
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
            GetScreenWidth() / 2
            -
            MeasureGameText(
                prompt,
                20
            )
            /
            2,
            GetScreenHeight() - 55,
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
                GetScreenWidth(),
                GetScreenHeight(),
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
                GetScreenWidth() / 2
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
                    GetScreenWidth() / 2
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
            GetScreenWidth() / 2
            -
            totalWidth / 2;


        // Correctly typed letters turn gold.
        DrawGameText(
            completed,
            startX,
            GetScreenHeight() - 83,
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
            GetScreenHeight() - 83,
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
            GetScreenWidth() / 2
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
                GetScreenWidth() / 2
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
            GetScreenWidth() / 2
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
            GetScreenHeight() - 48,
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


        chapterRestoredTimer =
            0;

        chapterRestoredNumber =
            -1;


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


        if (chapterRestoredTimer > 0)
        {
            chapterRestoredTimer--;
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


        const std::string displayedWord =
            enemy.GetWordDisplay();


        if (displayedWord.empty())
        {
            return;
        }


        const std::string typed =
            NormalizeTypingText(
                combat.GetInput()
            );


        // A ready Quill command gets visual priority over an
        // enemy word that happens to share the same prefix.
        if (
            !quillSystem
                .GetMatchingCommand(
                    combat.GetInput()
                )
                .empty()
        )
        {
            return;
        }


        if (typed.empty())
        {
            return;
        }


        const std::string target =
            NormalizeTypingText(
                displayedWord
            );


        if (
            typed.size()
            >
            target.size()
        )
        {
            return;
        }


        if (
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


        // Convert the visible portion to uppercase while
        // preserving apostrophes in the displayed target.
        std::string fullUpper =
            displayedWord;


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


        std::size_t visibleCharacters =
            0;


        std::size_t endIndex =
            0;


        while (
            endIndex
            <
            fullUpper.size()
            &&
            visibleCharacters
            <
            typed.size()
        )
        {
            const char character =
                fullUpper[
                    endIndex
                ];


            if (
                (
                    character >= 'A'
                    &&
                    character <= 'Z'
                )
                ||
                (
                    character >= 'a'
                    &&
                    character <= 'z'
                )
            )
            {
                visibleCharacters++;
            }


            endIndex++;
        }


        const std::string highlighted =
            fullUpper.substr(
                0,
                endIndex
            );


        constexpr int fontSize =
            27;


        const int fullWidth =
            MeasureGameText(
                fullUpper,
                fontSize
            );


        DrawGameText(
            highlighted,
            static_cast<int>(
                enemy.GetX()
            )
            -
            fullWidth / 2,
            static_cast<int>(
                enemy.GetY()
            )
            -
            62,
            fontSize,
            Color{
                255,
                215,
                70,
                255
            }
        );
    }


    void DrawAdventure()
    {
        ClearBackground(
            Color{
                15,
                15,
                25,
                255
            }
        );


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


        DrawGameText(
            progress.c_str(),
            20,
            88,
            22,
            Color{
                180,
                210,
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
                GetScreenHeight()
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
                GetScreenWidth(),
                GetScreenHeight(),
                true,
                combat.GetInput()
            );
        }


        if (
            chapterRestoredTimer > 0
            &&
            chapterRestoredNumber > 0
            &&
            !paused
            &&
            !gameOver
        )
        {
            const std::string chapterText =
                "CHAPTER "
                +
                std::to_string(
                    chapterRestoredNumber
                )
                +
                " RESTORED";


            DrawGameText(
                chapterText,
                GetScreenWidth() / 2
                -
                MeasureGameText(
                    chapterText,
                    34
                )
                /
                2,
                135,
                34,
                Color{
                    255,
                    225,
                    120,
                    255
                }
            );


            const char* libraryText =
                "The chapter can now be read in the Library.";


            DrawGameText(
                libraryText,
                GetScreenWidth() / 2
                -
                MeasureGameText(
                    libraryText,
                    20
                )
                /
                2,
                177,
                20,
                Color{
                    205,
                    205,
                    210,
                    255
                }
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
                GetScreenWidth() / 2
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
                GetScreenWidth(),
                GetScreenHeight(),
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
                GetScreenWidth() / 2
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
                    GetScreenWidth() / 2
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
                GetScreenWidth(),
                GetScreenHeight(),
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
                GetScreenWidth() / 2
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
                    GetScreenWidth() / 2
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
                GetScreenWidth() / 2
                -
                width / 2,
                GetScreenHeight() / 2,
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
    );


    InitWindow(
        900,
        600,
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


    ApplyDisplaySetting();


    storyReader =
        std::make_unique<
            StoryReader
        >(
            saveData.recoveredWordIds
        );


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
        if (IsWindowResized())
        {
            RefreshDisplayGeometry();
        }


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

        BeginDrawing();


        if (
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


    ShutdownGameFont();


    CloseWindow();


    return 0;
}