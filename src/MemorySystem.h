#pragma once

#include <string>
#include <vector>


struct MemoryFragment
{
    int threshold;
    const char* title;
    std::vector<std::string> lines;
};


class MemorySystem
{
public:
    MemorySystem();

    int GetMemoryCount() const;

    void SetViewportSize(
        int width,
        int height
    );

    int GetNextMemoryIndex(
        int wordsRecovered,
        int memoryStage
    ) const;

    void BeginPrologue();

    void BeginFragment(
        int index
    );

    // Returns true when the current memory has finished.
    bool HandleFragmentInput();

    void DrawFragment() const;

    // Memory archive. Returns true when ESC exits to menu.
    bool HandleLibraryInput(
        int memoryStage
    );

    void DrawLibrary(
        int memoryStage
    ) const;

    void ResetLibrary();


private:
    std::vector<MemoryFragment> memories;

    int activeMemory;
    int visibleLines;
    bool showingPrologue;
    bool showingQuillReveal;
    bool showingSafeEnding;

    int librarySelected;
    bool libraryReading;

    int viewportWidth;
    int viewportHeight;

    std::vector<std::string> prologueLines;

    const MemoryFragment* GetActiveFragment() const;
    bool IsQuillMemory() const;
    bool IsSafeMemory() const;

    void DrawQuillReveal() const;
    void DrawSafeEnding() const;

    void DrawCentered(
        const std::string& text,
        int y,
        int fontSize,
        unsigned char r,
        unsigned char g,
        unsigned char b
    )const;
};