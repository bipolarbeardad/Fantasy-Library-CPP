#include "StoryReader.h"
#include "GameFont.h"

#include "raylib.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>


StoryReader::StoryReader(
    const std::vector<int>& recoveredWordIds,
    const std::string& storyFile,
    const std::string& wordsFile
)
    :
    chapterIndex(0),
    pageIndex(0),
    loaded(false)
{
    SetRecoveredWords(
        recoveredWordIds
    );


    RecalculateGeometry();


    const bool storyLoaded =
        LoadStory(
            storyFile
        );


    const bool wordsLoaded =
        LoadWordLookup(
            wordsFile
        );


    loaded =
        storyLoaded
        &&
        wordsLoaded;
}


bool StoryReader::IsLoaded() const
{
    return loaded;
}


void StoryReader::SetRecoveredWords(
    const std::vector<int>& recoveredWordIds
)
{
    recoveredIds.clear();


    for (int id : recoveredWordIds)
    {
        recoveredIds.insert(
            id
        );
    }
}


void StoryReader::RecalculateGeometry()
{
    const int width =
        GetScreenWidth();


    const int height =
        GetScreenHeight();


    const int horizontalMargin =
        std::max(
            50,
            static_cast<int>(
                width * 0.07f
            )
        );


    pageLeft =
        horizontalMargin;


    pageRight =
        width
        -
        horizontalMargin;


    pageTop = 120;


    pageBottom =
        height
        -
        55;


    const int textMargin =
        std::max(
            32,
            static_cast<int>(
                width * 0.04f
            )
        );


    textLeft =
        pageLeft
        +
        textMargin;


    textRight =
        pageRight
        -
        textMargin;


    textTop =
        pageTop
        +
        64;


    textBottom =
        pageBottom
        -
        30;


    const int pageCount =
        GetPageCount();


    if (pageCount > 0)
    {
        pageIndex =
            std::min(
                pageIndex,
                pageCount - 1
            );
    }

    else
    {
        pageIndex = 0;
    }
}


bool StoryReader::HandleInput()
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        return true;
    }


    if (
        IsKeyPressed(KEY_A)
        ||
        IsKeyPressed(KEY_LEFT)
    )
    {
        PreviousPage();
    }


    else if (
        IsKeyPressed(KEY_D)
        ||
        IsKeyPressed(KEY_RIGHT)
    )
    {
        NextPage();
    }


    else if (
        IsKeyPressed(KEY_W)
        ||
        IsKeyPressed(KEY_UP)
    )
    {
        PreviousChapter();
    }


    else if (
        IsKeyPressed(KEY_S)
        ||
        IsKeyPressed(KEY_DOWN)
    )
    {
        NextChapter();
    }


    return false;
}


void StoryReader::NextPage()
{
    const int pageCount =
        GetPageCount();


    if (
        pageIndex + 1
        <
        pageCount
    )
    {
        pageIndex++;

        return;
    }


    if (
        chapterIndex + 1
        <
        static_cast<int>(
            chapters.size()
        )
    )
    {
        chapterIndex++;

        pageIndex = 0;
    }
}


void StoryReader::PreviousPage()
{
    if (pageIndex > 0)
    {
        pageIndex--;

        return;
    }


    if (chapterIndex > 0)
    {
        chapterIndex--;


        pageIndex =
            std::max(
                0,
                GetPageCount() - 1
            );
    }
}


void StoryReader::NextChapter()
{
    if (
        chapterIndex + 1
        <
        static_cast<int>(
            chapters.size()
        )
    )
    {
        chapterIndex++;

        pageIndex = 0;
    }
}


void StoryReader::PreviousChapter()
{
    if (chapterIndex > 0)
    {
        chapterIndex--;

        pageIndex = 0;
    }
}


bool StoryReader::WordIsRecovered(
    const std::string& word
) const
{
    const auto found =
        wordLookup.find(
            Lower(word)
        );


    // Words not in words.json stay visible.

    if (
        found
        ==
        wordLookup.end()
    )
    {
        return true;
    }


    return
        recoveredIds.find(
            found->second
        )
        !=
        recoveredIds.end();
}


std::string StoryReader::HideWord(
    const std::string& word
)
{
    std::string hidden;


    for (char character : word)
    {
        hidden +=
            character == '\''
            ?
            '\''
            :
            '_';
    }


    return hidden;
}


std::string StoryReader::Lower(
    std::string value
)
{
    for (char& character : value)
    {
        character =
            static_cast<char>(
                std::tolower(
                    static_cast<unsigned char>(
                        character
                    )
                )
            );
    }


    return value;
}


std::vector<StoryReader::Chunk>
StoryReader::ParagraphToChunks(
    const std::string& paragraph
) const
{
    std::vector<Chunk> chunks;


    std::size_t position = 0;


    while (
        position
        <
        paragraph.size()
    )
    {
        const unsigned char character =
            static_cast<unsigned char>(
                paragraph[position]
            );


        // Word begins.

        if (std::isalpha(character))
        {
            const std::size_t start =
                position;


            position++;


            while (
                position
                <
                paragraph.size()
            )
            {
                const unsigned char current =
                    static_cast<unsigned char>(
                        paragraph[position]
                    );


                if (std::isalpha(current))
                {
                    position++;

                    continue;
                }


                // Preserve apostrophes inside words.

                if (
                    paragraph[position] == '\''
                    &&
                    position + 1
                    <
                    paragraph.size()
                    &&
                    std::isalpha(
                        static_cast<unsigned char>(
                            paragraph[
                                position + 1
                            ]
                        )
                    )
                )
                {
                    position++;

                    continue;
                }


                break;
            }


            const std::string word =
                paragraph.substr(
                    start,
                    position - start
                );


            const bool recovered =
                WordIsRecovered(
                    word
                );


            chunks.push_back(
                Chunk{
                    recovered
                    ?
                    word
                    :
                    HideWord(word),

                    recovered
                }
            );


            continue;
        }


        // Non-word text.

        const std::size_t start =
            position;


        position++;


        while (
            position
            <
            paragraph.size()
            &&
            !std::isalpha(
                static_cast<unsigned char>(
                    paragraph[position]
                )
            )
        )
        {
            position++;
        }


        chunks.push_back(
            Chunk{
                paragraph.substr(
                    start,
                    position - start
                ),
                true
            }
        );
    }


    return chunks;
}


std::vector<StoryReader::Page>
StoryReader::LayoutChapter() const
{
    std::vector<Page> pages;


    if (
        chapters.empty()
        ||
        chapterIndex < 0
        ||
        chapterIndex
        >=
        static_cast<int>(
            chapters.size()
        )
    )
    {
        return pages;
    }


    constexpr int fontSize = 21;

    constexpr int lineHeight = 29;

    constexpr int paragraphGap = 11;


    const int maxWidth =
        textRight
        -
        textLeft;


    const int availableHeight =
        textBottom
        -
        textTop;


    Page currentPage;

    int currentY = 0;


    const Chapter& chapter =
        chapters[
            chapterIndex
        ];


    for (
        const std::string& paragraph
        :
        chapter.paragraphs
    )
    {
        const std::vector<Chunk> rawChunks =
            ParagraphToChunks(
                paragraph
            );


        std::vector<Line> lines;

        Line line;

        int lineWidth = 0;


        for (
            const Chunk& raw
            :
            rawChunks
        )
        {
            std::size_t start = 0;


            while (
                start
                <
                raw.text.size()
            )
            {
                // Preserve whitespace separately.

                if (
                    std::isspace(
                        static_cast<unsigned char>(
                            raw.text[start]
                        )
                    )
                )
                {
                    if (!line.empty())
                    {
                        line.push_back(
                            Chunk{
                                " ",
                                true
                            }
                        );


                        lineWidth +=
                            MeasureGameText(
                                " ",
                                fontSize
                            );
                    }


                    start++;

                    continue;
                }


                std::size_t end =
                    start;


                while (
                    end
                    <
                    raw.text.size()
                    &&
                    !std::isspace(
                        static_cast<unsigned char>(
                            raw.text[end]
                        )
                    )
                )
                {
                    end++;
                }


                const std::string piece =
                    raw.text.substr(
                        start,
                        end - start
                    );


                const int width =
                    MeasureGameText(
                        piece.c_str(),
                        fontSize
                    );


                if (
                    !line.empty()
                    &&
                    lineWidth + width
                    >
                    maxWidth
                )
                {
                    while (
                        !line.empty()
                        &&
                        line.back().text
                        ==
                        " "
                    )
                    {
                        line.pop_back();
                    }


                    lines.push_back(
                        line
                    );


                    line.clear();

                    lineWidth = 0;
                }


                line.push_back(
                    Chunk{
                        piece,
                        raw.recovered
                    }
                );


                lineWidth +=
                    width;


                start = end;
            }
        }


        while (
            !line.empty()
            &&
            line.back().text
            ==
            " "
        )
        {
            line.pop_back();
        }


        if (!line.empty())
        {
            lines.push_back(
                line
            );


            line.clear();

            lineWidth = 0;
        }


        for (
            const Line& lineData
            :
            lines
        )
        {
            if (
                currentY
                +
                lineHeight
                >
                availableHeight
            )
            {
                pages.push_back(
                    currentPage
                );


                currentPage.clear();

                currentY = 0;
            }


            currentPage.push_back(
                lineData
            );


            currentY +=
                lineHeight;
        }


        if (!lines.empty())
        {
            currentY +=
                paragraphGap;
        }
    }


    if (!currentPage.empty())
    {
        pages.push_back(
            currentPage
        );
    }


    if (pages.empty())
    {
        pages.push_back(
            Page{}
        );
    }


    return pages;
}


int StoryReader::GetPageCount() const
{
    if (chapters.empty())
    {
        return 0;
    }


    return static_cast<int>(
        LayoutChapter().size()
    );
}


void StoryReader::DrawPageBackground() const
{
    const int width =
        pageRight
        -
        pageLeft;


    const int height =
        pageBottom
        -
        pageTop;


    DrawRectangleRounded(
        Rectangle{
            static_cast<float>(
                pageLeft + 7
            ),
            static_cast<float>(
                pageTop + 8
            ),
            static_cast<float>(
                width
            ),
            static_cast<float>(
                height
            )
        },
        0.02f,
        4,
        Color{
            7,
            6,
            10,
            255
        }
    );


    DrawRectangleRounded(
        Rectangle{
            static_cast<float>(
                pageLeft
            ),
            static_cast<float>(
                pageTop
            ),
            static_cast<float>(
                width
            ),
            static_cast<float>(
                height
            )
        },
        0.02f,
        4,
        Color{
            232,
            220,
            185,
            255
        }
    );


    DrawRectangleRoundedLines(
        Rectangle{
            static_cast<float>(
                pageLeft
            ),
            static_cast<float>(
                pageTop
            ),
            static_cast<float>(
                width
            ),
            static_cast<float>(
                height
            )
        },
        0.02f,
        4,
        Color{
            150,
            128,
            92,
            255
        }
    );


    DrawRectangleLines(
        pageLeft + 9,
        pageTop + 9,
        width - 18,
        height - 18,
        Color{
            204,
            188,
            151,
            255
        }
    );
}


void StoryReader::DrawStoryLine(
    const Line& line,
    int y
) const
{
    int x =
        textLeft;


    for (
        const Chunk& chunk
        :
        line
    )
    {
        const Color color =
            chunk.recovered
            ?
            Color{
                55,
                45,
                38,
                255
            }
            :
            Color{
                150,
                135,
                112,
                255
            };


        DrawGameText(
            chunk.text.c_str(),
            x,
            y,
            21,
            color
        );


        x +=
            MeasureGameText(
                chunk.text.c_str(),
                21
            );
    }
}


void StoryReader::DrawCentered(
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
        GetScreenWidth() / 2
        -
        width / 2,
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


void StoryReader::Draw()
{
    RecalculateGeometry();


    ClearBackground(
        Color{
            18,
            15,
            25,
            255
        }
    );


    if (
        !loaded
        ||
        chapters.empty()
    )
    {
        DrawGameText(
            "No story data found.",
            50,
            100,
            27,
            RAYWHITE
        );


        return;
    }


    DrawCentered(
        title.empty()
        ?
        "The Tale of Bearly"
        :
        title,
        26,
        40,
        230,
        220,
        150
    );


    DrawCentered(
        "Play the game to unlock the story.",
        53,
        18,
        165,
        160,
        175
    );


    DrawPageBackground();


    const Chapter& chapter =
        chapters[
            chapterIndex
        ];


    const std::string heading =
        "Chapter "
        +
        std::to_string(
            chapter.number
        )
        +
        ": "
        +
        chapter.title;


    DrawCentered(
        heading,
        pageTop + 18,
        27,
        85,
        65,
        45
    );


    DrawLine(
        textLeft,
        pageTop + 51,
        textRight,
        pageTop + 45,
        Color{
            175,
            155,
            120,
            255
        }
    );


    const std::vector<Page> pages =
        LayoutChapter();


    if (
        pageIndex
        >=
        static_cast<int>(
            pages.size()
        )
    )
    {
        pageIndex =
            std::max(
                0,
                static_cast<int>(
                    pages.size()
                )
                -
                1
            );
    }


    const Page& page =
        pages[
            pageIndex
        ];


    constexpr int lineHeight = 29;


    for (
        int lineIndex = 0;
        lineIndex
        <
        static_cast<int>(
            page.size()
        );
        lineIndex++
    )
    {
        DrawStoryLine(
            page[lineIndex],
            textTop
            +
            lineIndex
            *
            lineHeight
        );
    }


    DrawCentered(
        std::to_string(
            pageIndex + 1
        ),
        pageBottom - 23,
        18,
        120,
        105,
        82
    );


    const std::string info =
        "Chapter "
        +
        std::to_string(
            chapterIndex + 1
        )
        +
        "/"
        +
        std::to_string(
            chapters.size()
        )
        +
        "   |   Page "
        +
        std::to_string(
            pageIndex + 1
        )
        +
        "/"
        +
        std::to_string(
            pages.size()
        );


    DrawGameText(
        info.c_str(),
        30,
        GetScreenHeight() - 25,
        18,
        Color{
            180,
            175,
            185,
            255
        }
    );


    const char* controls =
        "A/D or Left/Right Turn Page   W/S or Up/Down Chapter   ESC Menu";


    const int controlsWidth =
        MeasureGameText(
            controls,
            18
        );


    DrawGameText(
        controls,
        GetScreenWidth()
        -
        controlsWidth
        -
        30,
        GetScreenHeight() - 25,
        18,
        Color{
            180,
            175,
            185,
            255
        }
    );
}


bool StoryReader::LoadStory(
    const std::string& filePath
)
{
    chapters.clear();


    std::string text =
        ReadFile(
            filePath
        );


    if (text.empty())
    {
        return false;
    }


    text =
        NormalizePunctuation(
            text
        );


    title =
        ParseStringField(
            text,
            "t"
        );


    const std::size_t chaptersKey =
        text.find(
            "\"c\""
        );


    if (
        chaptersKey
        ==
        std::string::npos
    )
    {
        return false;
    }


    const std::size_t arrayStart =
        text.find(
            '[',
            chaptersKey
        );


    if (
        arrayStart
        ==
        std::string::npos
    )
    {
        return false;
    }


    const std::size_t arrayEnd =
        FindMatching(
            text,
            arrayStart,
            '[',
            ']'
        );


    if (
        arrayEnd
        ==
        std::string::npos
    )
    {
        return false;
    }


    std::size_t position =
        arrayStart + 1;


    while (
        position
        <
        arrayEnd
    )
    {
        const std::size_t objectStart =
            text.find(
                '{',
                position
            );


        if (
            objectStart
            ==
            std::string::npos
            ||
            objectStart
            >=
            arrayEnd
        )
        {
            break;
        }


        const std::size_t objectEnd =
            FindMatching(
                text,
                objectStart,
                '{',
                '}'
            );


        if (
            objectEnd
            ==
            std::string::npos
        )
        {
            break;
        }


        const std::string objectText =
            text.substr(
                objectStart,
                objectEnd
                -
                objectStart
                +
                1
            );


        Chapter chapter;


        chapter.number =
            ParseIntField(
                objectText,
                "n"
            );


        chapter.title =
            ParseStringField(
                objectText,
                "t"
            );


        chapter.paragraphs =
            ParseStringArray(
                objectText,
                "p"
            );


        if (chapter.number <= 0)
        {
            chapter.number =
                static_cast<int>(
                    chapters.size()
                )
                +
                1;
        }


        chapters.push_back(
            chapter
        );


        position =
            objectEnd + 1;
    }


    return !chapters.empty();
}


bool StoryReader::LoadWordLookup(
    const std::string& filePath
)
{
    wordLookup.clear();


    const std::string text =
        ReadFile(
            filePath
        );


    if (text.empty())
    {
        return false;
    }


    const std::size_t wordsKey =
        text.find(
            "\"words\""
        );


    if (
        wordsKey
        ==
        std::string::npos
    )
    {
        return false;
    }


    const std::size_t arrayStart =
        text.find(
            '[',
            wordsKey
        );


    if (
        arrayStart
        ==
        std::string::npos
    )
    {
        return false;
    }


    const std::size_t arrayEnd =
        FindMatching(
            text,
            arrayStart,
            '[',
            ']'
        );


    if (
        arrayEnd
        ==
        std::string::npos
    )
    {
        return false;
    }


    std::size_t position =
        arrayStart + 1;


    while (
        position
        <
        arrayEnd
    )
    {
        const std::size_t objectStart =
            text.find(
                '{',
                position
            );


        if (
            objectStart
            ==
            std::string::npos
            ||
            objectStart
            >=
            arrayEnd
        )
        {
            break;
        }


        const std::size_t objectEnd =
            FindMatching(
                text,
                objectStart,
                '{',
                '}'
            );


        if (
            objectEnd
            ==
            std::string::npos
        )
        {
            break;
        }


        const std::string objectText =
            text.substr(
                objectStart,
                objectEnd
                -
                objectStart
                +
                1
            );


        const int id =
            ParseIntField(
                objectText,
                "id"
            );


        const std::string word =
            ParseStringField(
                objectText,
                "word"
            );


        if (
            id >= 0
            &&
            !word.empty()
        )
        {
            wordLookup[
                Lower(word)
            ] =
                id;
        }


        position =
            objectEnd + 1;
    }


    return !wordLookup.empty();
}


std::string StoryReader::ReadFile(
    const std::string& filePath
)
{
    std::ifstream file(
        filePath,
        std::ios::binary
    );


    if (!file)
    {
        return "";
    }


    std::ostringstream buffer;

    buffer << file.rdbuf();


    return buffer.str();
}


std::size_t StoryReader::FindMatching(
    const std::string& text,
    std::size_t start,
    char openChar,
    char closeChar
)
{
    int depth = 0;

    bool inString = false;

    bool escaped = false;


    for (
        std::size_t index = start;
        index < text.size();
        index++
    )
    {
        const char character =
            text[index];


        if (inString)
        {
            if (escaped)
            {
                escaped = false;

                continue;
            }


            if (character == '\\')
            {
                escaped = true;

                continue;
            }


            if (character == '"')
            {
                inString = false;
            }


            continue;
        }


        if (character == '"')
        {
            inString = true;

            continue;
        }


        if (character == openChar)
        {
            depth++;
        }


        else if (
            character == closeChar
        )
        {
            depth--;


            if (depth == 0)
            {
                return index;
            }
        }
    }


    return std::string::npos;
}


int StoryReader::ParseIntField(
    const std::string& objectText,
    const std::string& fieldName
)
{
    const std::string key =
        "\""
        +
        fieldName
        +
        "\"";


    std::size_t position =
        objectText.find(
            key
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return -1;
    }


    position =
        objectText.find(
            ':',
            position + key.size()
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return -1;
    }


    position++;


    while (
        position
        <
        objectText.size()
        &&
        std::isspace(
            static_cast<unsigned char>(
                objectText[position]
            )
        )
    )
    {
        position++;
    }


    int value = 0;

    bool found = false;


    while (
        position
        <
        objectText.size()
        &&
        std::isdigit(
            static_cast<unsigned char>(
                objectText[position]
            )
        )
    )
    {
        found = true;


        value =
            value * 10
            +
            (
                objectText[position]
                -
                '0'
            );


        position++;
    }


    return found
        ?
        value
        :
        -1;
}


std::string StoryReader::ParseStringField(
    const std::string& objectText,
    const std::string& fieldName
)
{
    const std::string key =
        "\""
        +
        fieldName
        +
        "\"";


    std::size_t position =
        objectText.find(
            key
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return "";
    }


    position =
        objectText.find(
            ':',
            position + key.size()
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return "";
    }


    position =
        objectText.find(
            '"',
            position + 1
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return "";
    }


    position++;


    std::string value;

    bool escaped = false;


    while (
        position
        <
        objectText.size()
    )
    {
        const char character =
            objectText[position];


        if (escaped)
        {
            switch (character)
            {
                case 'n':
                    value += '\n';
                    break;

                case 'r':
                    value += '\r';
                    break;

                case 't':
                    value += '\t';
                    break;

                default:
                    value += character;
                    break;
            }


            escaped = false;

            position++;

            continue;
        }


        if (character == '\\')
        {
            escaped = true;

            position++;

            continue;
        }


        if (character == '"')
        {
            break;
        }


        value += character;

        position++;
    }


    return value;
}


std::vector<std::string>
StoryReader::ParseStringArray(
    const std::string& objectText,
    const std::string& fieldName
)
{
    std::vector<std::string> values;


    const std::string key =
        "\""
        +
        fieldName
        +
        "\"";


    const std::size_t keyPosition =
        objectText.find(
            key
        );


    if (
        keyPosition
        ==
        std::string::npos
    )
    {
        return values;
    }


    const std::size_t arrayStart =
        objectText.find(
            '[',
            keyPosition
        );


    if (
        arrayStart
        ==
        std::string::npos
    )
    {
        return values;
    }


    const std::size_t arrayEnd =
        FindMatching(
            objectText,
            arrayStart,
            '[',
            ']'
        );


    if (
        arrayEnd
        ==
        std::string::npos
    )
    {
        return values;
    }


    std::size_t position =
        arrayStart + 1;


    while (
        position
        <
        arrayEnd
    )
    {
        const std::size_t quoteStart =
            objectText.find(
                '"',
                position
            );


        if (
            quoteStart
            ==
            std::string::npos
            ||
            quoteStart
            >=
            arrayEnd
        )
        {
            break;
        }


        std::string value;

        bool escaped = false;


        std::size_t cursor =
            quoteStart + 1;


        while (
            cursor
            <
            arrayEnd
        )
        {
            const char character =
                objectText[cursor];


            if (escaped)
            {
                switch (character)
                {
                    case 'n':
                        value += '\n';
                        break;

                    case 'r':
                        value += '\r';
                        break;

                    case 't':
                        value += '\t';
                        break;

                    default:
                        value += character;
                        break;
                }


                escaped = false;

                cursor++;

                continue;
            }


            if (character == '\\')
            {
                escaped = true;

                cursor++;

                continue;
            }


            if (character == '"')
            {
                break;
            }


            value += character;

            cursor++;
        }


        values.push_back(
            NormalizePunctuation(
                value
            )
        );


        position =
            cursor + 1;
    }


    return values;
}


std::string StoryReader::NormalizePunctuation(
    std::string text
)
{
    // Curly apostrophes -> straight apostrophe.
    const std::string curlyApostrophe =
        "\xE2\x80\x99";


    std::size_t position = 0;


    while (
        (
            position =
                text.find(
                    curlyApostrophe,
                    position
                )
        )
        !=
        std::string::npos
    )
    {
        text.replace(
            position,
            curlyApostrophe.size(),
            "'"
        );


        position++;
    }


    // Em dash -> ASCII dash.
    const std::string emDash =
        "\xE2\x80\x94";


    position = 0;


    while (
        (
            position =
                text.find(
                    emDash,
                    position
                )
        )
        !=
        std::string::npos
    )
    {
        text.replace(
            position,
            emDash.size(),
            "-"
        );


        position++;
    }


    // Left/right curly double quotes -> straight quote.
    const std::string leftQuote =
        "\xE2\x80\x9C";


    const std::string rightQuote =
        "\xE2\x80\x9D";


    for (
        const std::string* mark
        :
        {
            &leftQuote,
            &rightQuote
        }
    )
    {
        position = 0;


        while (
            (
                position =
                    text.find(
                        *mark,
                        position
                    )
            )
            !=
            std::string::npos
        )
        {
            text.replace(
                position,
                mark->size(),
                "\""
            );


            position++;
        }
    }


    return text;
}