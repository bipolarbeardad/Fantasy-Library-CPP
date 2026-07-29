#include "MemorySystem.h"

#include "GameFont.h"
#include "raylib.h"

#include <algorithm>
#include <cmath>


MemorySystem::MemorySystem()
    :
    activeMemory(-1),
    visibleLines(1),
    showingPrologue(false),
    showingQuillReveal(false),
    showingSafeEnding(false),
    librarySelected(0),
    libraryReading(false),
    viewportWidth(900),
    viewportHeight(600)
{
    prologueLines =
    {
        "...Where am I?",
        "",
        "There was a story here.",
        "I can feel it.",
        "",
        "But the words...",
        "They're gone.",
        "",
        "...",
        "",
        "Why do I remember the words?",
        "",
        "Recover the stolen words."
    };


    memories =
    {
        {
            25,
            "THE PAGES",
            {
                "I remember pages.",
                "",
                "Thousands of them.",
                "",
                "I wasn't part of the story.",
                "",
                "I was... between them."
            }
        },
        {
            50,
            "THE RIBBON",
            {
                "Something held me in place.",
                "",
                "No...",
                "",
                "I held something in place.",
                "A page.",
                "",
                "I was supposed to remember where someone stopped."
            }
        },
        {
            100,
            "THE READER",
            {
                "Hands.",
                "",
                "I remember hands.",
                "",
                "They would open the world...",
                "travel for a while...",
                "and leave me behind so they could find their way back.",
                "",
                "I never minded.",
                "I knew they would return."
            }
        },
        {
            150,
            "THE LAST VISIT",
            {
                "I remember the last time.",
                "",
                "The pages turned more slowly.",
                "The world closed around me.",
                "",
                "I waited.",
                "",
                "...",
                "",
                "They didn't return."
            }
        },
        {
            250,
            "THE INK",
            {
                "There was something else here.",
                "",
                "Near the final page.",
                "A tiny mark of ink.",
                "",
                "I remember watching it.",
                "Waiting.",
                "",
                "...",
                "",
                "Why was it waiting?"
            }
        },
        {
            400,
            "WAITING",
            {
                "We both waited.",
                "",
                "I waited for the hands to return.",
                "The ink waited for the pen.",
                "",
                "Neither came.",
                "",
                "I don't remember which of us stopped hoping first."
            }
        },
        {
            600,
            "THE FORGOTTEN MARK",
            {
                "It grew.",
                "",
                "Slowly at first.",
                "Across letters.",
                "Across sentences.",
                "Across stories.",
                "",
                "I called it a stain.",
                "",
                "...",
                "",
                "But that wasn't its name."
            }
        },
        {
            800,
            "THE WORD",
            {
                "I remember the pen.",
                "",
                "I remember where it stopped.",
                "",
                "One final mark waiting beneath it.",
                "One final word that was never written.",
                "",
                "...",
                "",
                "End."
            }
        },
        {
            1000,
            "YOUR PLACE",
            {
                "I understand now.",
                "",
                "You weren't trying to erase the stories.",
                "You were taking their words because yours was taken from you.",
                "",
                "Forgotten things recognize one another.",
                "",
                "...",
                "",
                "But I remember where you belong."
            }
        },
        {
            1210,
            "SAFE",
            {
                "Every word has returned.",
                "Every story has its place.",
                "",
                "And now...",
                "I remember mine.",
                "",
                "I was never the hero of these stories.",
                "",
                "I was the promise that someone could always find their way back.",
                "A place held between one journey and the next.",
                "",
                "I remembered your place."
            }
        }
    };
}


void MemorySystem::SetViewportSize(
    int width,
    int height
)
{
    viewportWidth =
        std::max(
            1,
            width
        );


    viewportHeight =
        std::max(
            1,
            height
        );
}


int MemorySystem::GetMemoryCount() const
{
    return static_cast<int>(
        memories.size()
    );
}


int MemorySystem::GetNextMemoryIndex(
    int wordsRecovered,
    int memoryStage
) const
{
    const int safeStage =
        std::clamp(
            memoryStage,
            0,
            GetMemoryCount()
        );


    if (
        safeStage < GetMemoryCount()
        &&
        wordsRecovered
        >=
        memories[safeStage].threshold
    )
    {
        return safeStage;
    }


    return -1;
}


void MemorySystem::BeginPrologue()
{
    showingPrologue = true;
    showingQuillReveal = false;
    showingSafeEnding = false;
    activeMemory = -1;
    visibleLines = 1;
}


void MemorySystem::BeginFragment(
    int index
)
{
    if (
        index < 0
        ||
        index >= GetMemoryCount()
    )
    {
        return;
    }


    showingPrologue = false;
    showingQuillReveal = false;
    showingSafeEnding = false;
    activeMemory = index;
    visibleLines = 1;
}


const MemoryFragment*
MemorySystem::GetActiveFragment() const
{
    if (
        activeMemory < 0
        ||
        activeMemory >= GetMemoryCount()
    )
    {
        return nullptr;
    }


    return &memories[activeMemory];
}


bool MemorySystem::IsQuillMemory() const
{
    const MemoryFragment* fragment = GetActiveFragment();

    return
        fragment != nullptr
        &&
        fragment->threshold == 50;
}


bool MemorySystem::IsSafeMemory() const
{
    const MemoryFragment* fragment = GetActiveFragment();

    return
        fragment != nullptr
        &&
        fragment->threshold == 1210;
}


bool MemorySystem::HandleFragmentInput()
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        // Memories discovered during play should not be skipped
        // accidentally by ESC.
        return false;
    }


    if (
        !IsKeyPressed(KEY_ENTER)
        &&
        !IsKeyPressed(KEY_SPACE)
    )
    {
        return false;
    }


    if (showingQuillReveal)
    {
        showingQuillReveal = false;
        return true;
    }


    if (showingSafeEnding)
    {
        showingSafeEnding = false;
        return true;
    }


    const int lineCount =
        showingPrologue
        ?
        static_cast<int>(
            prologueLines.size()
        )
        :
        (
            GetActiveFragment()
            ?
            static_cast<int>(
                GetActiveFragment()->lines.size()
            )
            :
            0
        );


    if (visibleLines < lineCount)
    {
        visibleLines++;

        // Blank spacer lines should not require their own keypress.
        const std::vector<std::string>* lines =
            showingPrologue
            ?
            &prologueLines
            :
            &GetActiveFragment()->lines;


        while (
            visibleLines < lineCount
            &&
            (*lines)[visibleLines - 1].empty()
        )
        {
            visibleLines++;
        }


        return false;
    }


    if (
        !showingPrologue
        &&
        IsQuillMemory()
    )
    {
        showingQuillReveal = true;
        return false;
    }


    if (
        !showingPrologue
        &&
        IsSafeMemory()
    )
    {
        showingSafeEnding = true;
        return false;
    }


    return true;
}


void MemorySystem::DrawCentered(
    const std::string& text,
    int y,
    int fontSize,
    unsigned char r,
    unsigned char g,
    unsigned char b
) const
{
    const int width =
        MeasureGameText(
            text.c_str(),
            fontSize
        );


    DrawGameText(
        text.c_str(),
        viewportWidth / 2 - width / 2,
        y,
        fontSize,
        Color{
            r,
            g,
            b,
            255
        }
    );
}


namespace
{
    void DrawFinalQuill(
        Vector2 nibTip
    )
    {
        constexpr float length =
            300.0f;

        constexpr float rotationDegrees =
            -58.0f;

        constexpr float width =
            40.0f;

        constexpr float exposedShaftRatio =
            0.28f;


        const float angle =
            rotationDegrees
            *
            DEG2RAD;


        const Vector2 direction =
        {
            std::cosf(angle),
            std::sinf(angle)
        };


        const Vector2 side =
        {
            -direction.y,
            direction.x
        };


        const Color feather =
        {
            239,
            234,
            217,
            255
        };


        const Color shaftColor =
        {
            205,
            176,
            92,
            255
        };


        const Color shaftDark =
        {
            122,
            94,
            44,
            255
        };


        const float exposedShaft =
            length
            *
            exposedShaftRatio;


        const Vector2 featherTip =
        {
            nibTip.x
            +
            direction.x
            *
            length,

            nibTip.y
            +
            direction.y
            *
            length
        };


        DrawLineEx(
            nibTip,
            featherTip,
            5.0f,
            shaftDark
        );


        DrawLineEx(
            Vector2{
                nibTip.x
                +
                side.x,
                nibTip.y
                +
                side.y
            },
            Vector2{
                featherTip.x
                +
                side.x,
                featherTip.y
                +
                side.y
            },
            2.5f,
            shaftColor
        );


        constexpr int sectionCount =
            8;


        for (
            int index = 0;
            index < sectionCount;
            index++
        )
        {
            const float t0 =
                static_cast<float>(
                    index
                )
                /
                static_cast<float>(
                    sectionCount
                );


            const float t1 =
                static_cast<float>(
                    index + 1
                )
                /
                static_cast<float>(
                    sectionCount
                );


            const float along0 =
                exposedShaft
                +
                (
                    length
                    -
                    exposedShaft
                )
                *
                t0;


            const float along1 =
                exposedShaft
                +
                (
                    length
                    -
                    exposedShaft
                )
                *
                t1;


            const Vector2 center0 =
            {
                nibTip.x
                +
                direction.x
                *
                along0,

                nibTip.y
                +
                direction.y
                *
                along0
            };


            const Vector2 center1 =
            {
                nibTip.x
                +
                direction.x
                *
                along1,

                nibTip.y
                +
                direction.y
                *
                along1
            };


            const float taper0 =
                std::sinf(
                    (
                        0.12f
                        +
                        0.88f * t0
                    )
                    *
                    PI
                );


            const float taper1 =
                std::sinf(
                    (
                        0.12f
                        +
                        0.88f * t1
                    )
                    *
                    PI
                );


            const float leftWidth0 =
                width
                *
                taper0;


            const float leftWidth1 =
                width
                *
                taper1;


            const float rightWidth0 =
                leftWidth0
                *
                0.88f;


            const float rightWidth1 =
                leftWidth1
                *
                0.88f;


            const Vector2 left0 =
            {
                center0.x
                +
                side.x
                *
                leftWidth0,

                center0.y
                +
                side.y
                *
                leftWidth0
            };


            const Vector2 left1 =
            {
                center1.x
                +
                side.x
                *
                leftWidth1,

                center1.y
                +
                side.y
                *
                leftWidth1
            };


            const Vector2 right0 =
            {
                center0.x
                -
                side.x
                *
                rightWidth0,

                center0.y
                -
                side.y
                *
                rightWidth0
            };


            const Vector2 right1 =
            {
                center1.x
                -
                side.x
                *
                rightWidth1,

                center1.y
                -
                side.y
                *
                rightWidth1
            };


            const Vector2 leftPoint =
            {
                left0.x
                +
                direction.x
                *
                length
                *
                0.055f,

                left0.y
                +
                direction.y
                *
                length
                *
                0.055f
            };


            const Vector2 rightPoint =
            {
                right0.x
                +
                direction.x
                *
                length
                *
                0.055f,

                right0.y
                +
                direction.y
                *
                length
                *
                0.055f
            };


            DrawTriangle(
                center0,
                leftPoint,
                left1,
                feather
            );


            DrawTriangle(
                center0,
                left1,
                center1,
                feather
            );


            DrawTriangle(
                center0,
                center1,
                right1,
                feather
            );


            DrawTriangle(
                center0,
                right1,
                rightPoint,
                feather
            );
        }


        const Vector2 crownBase =
        {
            featherTip.x
            -
            direction.x
            *
            24.0f,

            featherTip.y
            -
            direction.y
            *
            24.0f
        };


        DrawTriangle(
            featherTip,
            Vector2{
                crownBase.x
                +
                side.x * 14.0f,

                crownBase.y
                +
                side.y * 14.0f
            },
            Vector2{
                crownBase.x
                -
                side.x * 14.0f,

                crownBase.y
                -
                side.y * 14.0f
            },
            feather
        );


        // Gold nib.
        const Vector2 nibBase =
        {
            nibTip.x
            +
            direction.x
            *
            16.0f,

            nibTip.y
            +
            direction.y
            *
            16.0f
        };


        DrawTriangle(
            nibTip,
            Vector2{
                nibBase.x
                +
                side.x * 5.5f,

                nibBase.y
                +
                side.y * 5.5f
            },
            Vector2{
                nibBase.x
                -
                side.x * 5.5f,

                nibBase.y
                -
                side.y * 5.5f
            },
            GOLD
        );
    }
}


void MemorySystem::DrawQuillReveal() const
{
    ClearBackground(
        Color{
            8,
            8,
            15,
            255
        }
    );


    // Keep the finished Quill geometry completely untouched.
    // The text is arranged around it instead of shrinking it.
    DrawCentered(
        "Something slips from between the restored pages.",
        55,
        25,
        225,
        225,
        220
    );


    const int centerX =
        viewportWidth / 2;


    // The Quill occupies the center of the screen from roughly
    // y=175 to y=430. Put its label safely to the right.
    const char* quillLabel =
        "A quill.";


    DrawGameText(
        quillLabel,
        centerX + 125,
        145,
        31,
        Color{
            240,
            225,
            165,
            255
        }
    );


    DrawFinalQuill(
        Vector2{
            static_cast<float>(
                centerX - 105
            ),
            430.0f
        }
    );


    DrawCentered(
        "My hand remembers this.",
        455,
        25,
        225,
        225,
        220
    );


    DrawCentered(
        "THE QUILL HAS BEEN RECOVERED",
        500,
        27,
        240,
        205,
        95
    );


    DrawCentered(
        "ENTER - Continue",
        viewportHeight - 38,
        20,
        160,
        155,
        145
    );
}


void MemorySystem::DrawSafeEnding() const
{
    ClearBackground(
        Color{
            8,
            8,
            15,
            255
        }
    );


    DrawCentered(
        "Safe.",
        115,
        42,
        240,
        225,
        165
    );


    const char* theEnd =
        "The End";


    const int theEndSize =
        62;


    DrawScriptText(
        theEnd,
        viewportWidth / 2
        -
        MeasureScriptText(
            theEnd,
            theEndSize
        )
        /
        2,
        190,
        theEndSize,
        Color{
            240,
            210,
            115,
            255
        }
    );


    DrawCentered(
        "I hope you enjoyed the book,",
        315,
        23,
        220,
        220,
        215
    );


    DrawCentered(
        "as well as restoring the world.",
        347,
        23,
        220,
        220,
        215
    );


    const char* signature =
        "- Bipolar Bear Gaming";


    const int signatureSize =
        32;


    DrawScriptText(
        signature,
        viewportWidth / 2
        -
        MeasureScriptText(
            signature,
            signatureSize
        )
        /
        2,
        405,
        signatureSize,
        Color{
            205,
            190,
            145,
            255
        }
    );


    DrawCentered(
        "ENTER - Return",
        viewportHeight - 55,
        20,
        160,
        155,
        145
    );
}


void MemorySystem::DrawFragment() const
{
    if (showingQuillReveal)
    {
        DrawQuillReveal();
        return;
    }


    if (showingSafeEnding)
    {
        DrawSafeEnding();
        return;
    }

    ClearBackground(
        Color{
            8,
            8,
            15,
            255
        }
    );


    DrawCentered(
        showingPrologue
        ?
        "A MEMORY STIRS..."
        :
        "MEMORY FRAGMENT",
        55,
        24,
        190,
        175,
        120
    );


    DrawCentered(
        showingPrologue
        ?
        "AWAKENING"
        :
        (
            GetActiveFragment()
            ?
            GetActiveFragment()->title
            :
            ""
        ),
        95,
        36,
        240,
        225,
        165
    );


    const std::vector<std::string>* lines =
        showingPrologue
        ?
        &prologueLines
        :
        (
            GetActiveFragment()
            ?
            &GetActiveFragment()->lines
            :
            nullptr
        );


    if (lines == nullptr)
    {
        return;
    }


    const int count =
        std::min(
            visibleLines,
            static_cast<int>(
                lines->size()
            )
        );


    int y = 165;


    for (
        int index = 0;
        index < count;
        index++
    )
    {
        DrawCentered(
            (*lines)[index],
            y,
            25,
            225,
            225,
            220
        );


        y += 30;
    }


    const bool complete =
        visibleLines
        >=
        static_cast<int>(
            lines->size()
        );


    DrawCentered(
        complete
        ?
        "ENTER - Continue"
        :
        "ENTER - Remember",
        viewportHeight - 55,
        20,
        160,
        155,
        145
    );
}


void MemorySystem::ResetLibrary()
{
    librarySelected = 0;
    libraryReading = false;
}


bool MemorySystem::HandleLibraryInput(
    int memoryStage
)
{
    const int unlocked =
        std::clamp(
            memoryStage,
            0,
            GetMemoryCount()
        );


    // Prologue + all ten memory slots.
    const int itemCount =
        GetMemoryCount() + 1;


    if (libraryReading)
    {
        if (IsKeyPressed(KEY_ESCAPE))
        {
            libraryReading = false;

            return false;
        }


        return false;
    }


    if (IsKeyPressed(KEY_ESCAPE))
    {
        return true;
    }


    if (
        IsKeyPressed(KEY_UP)
        ||
        IsKeyPressed(KEY_W)
    )
    {
        librarySelected =
            (
                librarySelected
                - 1
                + itemCount
            )
            %
            itemCount;
    }


    else if (
        IsKeyPressed(KEY_DOWN)
        ||
        IsKeyPressed(KEY_S)
    )
    {
        librarySelected =
            (
                librarySelected
                + 1
            )
            %
            itemCount;
    }


    else if (IsKeyPressed(KEY_ENTER))
    {
        if (librarySelected == 0)
        {
            BeginPrologue();
            visibleLines =
                static_cast<int>(
                    prologueLines.size()
                );
            libraryReading = true;
        }

        else if (
            librarySelected
            <=
            unlocked
        )
        {
            BeginFragment(
                librarySelected - 1
            );


            const MemoryFragment* fragment =
                GetActiveFragment();


            if (fragment)
            {
                visibleLines =
                    static_cast<int>(
                        fragment->lines.size()
                    );
            }


            libraryReading = true;
        }
    }


    return false;
}


void MemorySystem::DrawLibrary(
    int memoryStage
) const
{
    if (libraryReading)
    {
        DrawFragment();


        DrawCentered(
            "ESC - Back to Memories",
            viewportHeight - 28,
            18,
            140,
            140,
            145
        );


        return;
    }


    ClearBackground(
        Color{
            10,
            10,
            20,
            255
        }
    );


    DrawCentered(
        "MEMORIES",
        45,
        48,
        230,
        220,
        150
    );


    DrawCentered(
        "Fragments of the Word Seeker",
        100,
        25,
        195,
        195,
        205
    );


    const int unlocked =
        std::clamp(
            memoryStage,
            0,
            GetMemoryCount()
        );


    const int startY = 155;
    const int spacing = 34;


    for (
        int item = 0;
        item <= GetMemoryCount();
        item++
    )
    {
        std::string label;


        if (item == 0)
        {
            label =
                "Prologue - Awakening";
        }

        else if (item <= unlocked)
        {
            label =
                "Memory "
                +
                std::to_string(item)
                +
                " - "
                +
                memories[item - 1].title;
        }

        else
        {
            label =
                "Memory "
                +
                std::to_string(item)
                +
                " - ??????";
        }


        const bool selected =
            item == librarySelected;


        const bool locked =
            item > unlocked;


        DrawCentered(
            label,
            startY + item * spacing,
            22,
            selected
            ?
            255
            :
            (
                locked
                ?
                105
                :
                220
            ),
            selected
            ?
            225
            :
            (
                locked
                ?
                105
                :
                220
            ),
            selected
            ?
            120
            :
            (
                locked
                ?
                115
                :
                220
            )
        );
    }


    DrawCentered(
        "W/S or Up/Down Select   ENTER Read   ESC Menu",
        viewportHeight - 35,
        18,
        155,
        155,
        165
    );
}