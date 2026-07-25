#include "raylib.h"

#include "Combat.h"
#include "Enemy.h"
#include "GameFont.h"
#include "MainMenu.h"
#include "SaveManager.h"
#include "StoryReader.h"
#include "WordManager.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace
{
    constexpr int FPS = 60;
    constexpr int MAX_HEALTH = 3;

    enum class GameState
    {
        Menu,
        Adventure,
        Story
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


    bool paused = false;

    int pauseSelected = 0;


    bool gameOver = false;

    int gameOverSelected = 0;


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
    }


    int GetWaveSize()
    {
        const int recovered =
            saveData.wordsRecovered;


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


        return GetRandomValue(
            2,
            5
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
        int count
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
                    .GetWords(
                        excluded
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


        const std::vector<WordRecord>
            records =
                GetWordRecords(
                    wordCount
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
        }


        enemies.emplace_back(
            words,
            wordIds,
            wordManager.GetEnemyType(
                saveData.wordsRecovered
            ),
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


        return true;
    }


    void StartWave()
    {
        enemies.clear();

        reservedWordIds.clear();

        combat.ClearInput();

        waveClearTimer = 0;


        const int waveSize =
            GetWaveSize();


        std::array<int, 5> lanes =
        {
            0,
            1,
            2,
            3,
            4
        };


        // Fisher-Yates shuffle.

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
            int index = 0;
            index < waveSize;
            index++
        )
        {
            CreateEnemy(
                lanes[index]
            );
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


    void EnemyReachedPlayer(
        Enemy& enemy
    )
    {
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


        combat.ClearInput();


        if (playerHealth <= 0)
        {
            playerHealth = 0;

            gameOver = true;

            gameOverSelected = 0;
        }
    }


    void NewGame()
    {
        saveData
            .recoveredWordIds
            .clear();


        saveData.wordsRecovered = 0;


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


        if (!enemies.empty())
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


        if (waveClearTimer <= 0)
        {
            waveClearTimer = 90;
        }

        else
        {
            waveClearTimer--;


            if (waveClearTimer <= 0)
            {
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
            "Words: "
            +
            std::to_string(
                saveData.wordsRecovered
            )
            +
            "/"
            +
            std::to_string(
                wordManager
                .GetTotalWordCount()
            )
            +
            "    Wave: "
            +
            std::to_string(
                waveNumber
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


            DrawGameText(
                typed.c_str(),
                playerX
                -
                textWidth / 2,
                GetScreenHeight()
                -
                45,
                32,
                Color{
                    255,
                    230,
                    90,
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


            case MenuAction::NewGame:
            {
                NewGame();

                currentState =
                    GameState::Adventure;


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
        FLAG_WINDOW_HIGHDPI
    );


    InitWindow(
        900,
        600,
        "Fantasy Library: Quest of the Word Seeker"
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