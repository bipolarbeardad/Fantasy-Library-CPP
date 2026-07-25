#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


class StoryReader
{
public:

    StoryReader(
        const std::vector<int>& recoveredWordIds,
        const std::string& storyFile = "bear_story.json",
        const std::string& wordsFile = "words.json"
    );


    bool IsLoaded() const;


    void SetRecoveredWords(
        const std::vector<int>& recoveredWordIds
    );


    bool HandleInput();

    void Draw();


private:

    struct Chapter
    {
        int number = 0;
        std::string title;
        std::vector<std::string> paragraphs;
    };


    struct Chunk
    {
        std::string text;
        bool recovered = true;
    };


    using Line =
        std::vector<Chunk>;


    using Page =
        std::vector<Line>;


    std::string title;

    std::vector<Chapter> chapters;

    std::unordered_map<
        std::string,
        int
    > wordLookup;

    std::unordered_set<int>
        recoveredIds;


    int chapterIndex;

    int pageIndex;

    bool loaded;


    int pageLeft;
    int pageRight;
    int pageTop;
    int pageBottom;

    int textLeft;
    int textRight;
    int textTop;
    int textBottom;


    void RecalculateGeometry();


    bool LoadStory(
        const std::string& filePath
    );


    bool LoadWordLookup(
        const std::string& filePath
    );


    bool WordIsRecovered(
        const std::string& word
    ) const;


    static std::string HideWord(
        const std::string& word
    );


    static std::string Lower(
        std::string value
    );


    std::vector<Chunk>
    ParagraphToChunks(
        const std::string& paragraph
    ) const;


    std::vector<Page>
    LayoutChapter() const;


    int GetPageCount() const;


    void NextPage();

    void PreviousPage();

    void NextChapter();

    void PreviousChapter();


    void DrawPageBackground() const;

    void DrawStoryLine(
        const Line& line,
        int y
    ) const;


    static std::string ReadFile(
        const std::string& filePath
    );


    static std::size_t FindMatching(
        const std::string& text,
        std::size_t start,
        char openChar,
        char closeChar
    );


    static int ParseIntField(
        const std::string& objectText,
        const std::string& fieldName
    );


    static std::string ParseStringField(
        const std::string& objectText,
        const std::string& fieldName
    );


    static std::vector<std::string>
    ParseStringArray(
        const std::string& objectText,
        const std::string& fieldName
    );


    static std::string NormalizePunctuation(
        std::string text
    );


    static void DrawCentered(
        const std::string& text,
        int y,
        int fontSize,
        unsigned char r,
        unsigned char g,
        unsigned char b
    );
};