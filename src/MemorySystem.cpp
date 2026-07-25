#include "MemorySystem.h"

#include "GameFont.h"
#include "raylib.h"

#include <algorithm>


MemorySystem::MemorySystem()
    :
    activeMemory(-1),
    visibleLines(1),
    showingPrologue(false),
    librarySelected(0),
    libraryReading(false)
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
                "I was supposed to remember where someone stopped.",
                "",
                "...",
                "",
                "Something slips from between the restored pages.",
                "",
                "A quill.",
                "",
                "My hand remembers this.",
                "",
                "THE QUILL HAS BEEN RECOVERED"
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
                "...",
                "",
                "I remembered your place.",
                "",
                "Safe."
            }
        }
    };
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


    return true;
}


void MemorySystem::DrawCentered(
    const std::string& text,
    int y,
    int fontSize,
    unsigned char r,
    unsigned char g,
    unsigned char b
)
{
    const int width =
        MeasureGameText(
            text.c_str(),
            fontSize
        );


    DrawGameText(
        text.c_str(),
        GetScreenWidth() / 2 - width / 2,
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


void MemorySystem::DrawFragment() const
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
        GetScreenHeight() - 55,
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
            GetScreenHeight() - 28,
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
        GetScreenHeight() - 35,
        18,
        155,
        155,
        165
    );
}