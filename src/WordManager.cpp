#include "WordManager.h"

#include "raylib.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>


WordManager::WordManager(
    const std::string& filePath
)
{
    loaded =
        LoadWords(
            filePath
        );
}


bool WordManager::IsLoaded() const
{
    return loaded;
}


int WordManager::GetTotalWordCount() const
{
    return static_cast<int>(
        words.size()
    );
}


bool WordManager::ContainsId(
    const std::vector<int>& ids,
    int id
)
{
    return
        std::find(
            ids.begin(),
            ids.end(),
            id
        )
        !=
        ids.end();
}


int WordManager::GameplayChapter(
    int firstChapter
)
{
    // words.json assigns the title words to chapter 0.
    // For gameplay they join Chapter 1 so nothing gets stranded.
    return
        firstChapter <= 1
        ?
        1
        :
        firstChapter;
}


int WordManager::GetCurrentChapter(
    const std::vector<int>& recoveredWordIds
) const
{
    int currentChapter =
        -1;


    for (
        const WordRecord& record
        :
        words
    )
    {
        if (
            ContainsId(
                recoveredWordIds,
                record.id
            )
        )
        {
            continue;
        }


        const int chapter =
            GameplayChapter(
                record.firstChapter
            );


        if (
            currentChapter < 0
            ||
            chapter < currentChapter
        )
        {
            currentChapter =
                chapter;
        }
    }


    return currentChapter;
}


bool WordManager::IsChapterComplete(
    int chapter,
    const std::vector<int>& recoveredWordIds
) const
{
    for (
        const WordRecord& record
        :
        words
    )
    {
        if (
            GameplayChapter(
                record.firstChapter
            )
            !=
            chapter
        )
        {
            continue;
        }


        if (
            !ContainsId(
                recoveredWordIds,
                record.id
            )
        )
        {
            return false;
        }
    }


    return true;
}


std::vector<WordRecord>
WordManager::BuildCurrentChapterPool(
    const std::vector<int>& excludedWordIds
) const
{
    int chapter =
        -1;


    // Determine the earliest chapter containing a word that has
    // not been recovered/reserved by the caller.
    for (
        const WordRecord& record
        :
        words
    )
    {
        if (
            ContainsId(
                excludedWordIds,
                record.id
            )
        )
        {
            continue;
        }


        const int recordChapter =
            GameplayChapter(
                record.firstChapter
            );


        if (
            chapter < 0
            ||
            recordChapter < chapter
        )
        {
            chapter =
                recordChapter;
        }
    }


    if (chapter < 0)
    {
        return {};
    }


    std::vector<WordRecord>
        pool;


    for (
        const WordRecord& record
        :
        words
    )
    {
        if (
            ContainsId(
                excludedWordIds,
                record.id
            )
        )
        {
            continue;
        }


        if (
            GameplayChapter(
                record.firstChapter
            )
            ==
            chapter
        )
        {
            pool.push_back(
                record
            );
        }
    }


    return pool;
}


bool WordManager::IsCommonWord(
    const std::string& word
)
{
    // Deliberately small list of extremely common English words.
    // Goblins exhaust these first within the active chapter.
    static const char* COMMON_WORDS[] =
    {
        "the",
        "and",
        "of",
        "to",
        "a",
        "in",
        "is",
        "it",
        "he",
        "she",
        "was",
        "were",
        "for",
        "with",
        "on",
        "as",
        "at",
        "by",
        "be",
        "this",
        "that",
        "but",
        "or",
        "an",
        "his",
        "her",
        "they",
        "you",
        "we",
        "i",
        "not",
        "from",
        "had",
        "has",
        "have"
    };


    std::string lower =
        word;


    for (
        char& character
        :
        lower
    )
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


    for (
        const char* common
        :
        COMMON_WORDS
    )
    {
        if (lower == common)
        {
            return true;
        }
    }


    return false;
}


std::vector<WordRecord>
WordManager::BuildPriorityPool(
    const std::vector<WordRecord>& chapterPool,
    EnemyType enemyType
) const
{
    if (chapterPool.empty())
    {
        return {};
    }


    std::vector<WordRecord>
        preferred;


    if (enemyType == EnemyType::Goblin)
    {
        // Goblins first consume the common words in the current
        // chapter. Only after those are gone do they move to the
        // other small words.
        for (
            const WordRecord& record
            :
            chapterPool
        )
        {
            if (
                IsCommonWord(
                    record.word
                )
            )
            {
                preferred.push_back(
                    record
                );
            }
        }


        if (!preferred.empty())
        {
            return preferred;
        }


        for (
            const WordRecord& record
            :
            chapterPool
        )
        {
            if (record.length <= 5)
            {
                preferred.push_back(
                    record
                );
            }
        }


        if (!preferred.empty())
        {
            return preferred;
        }


        // If only longer words remain in the chapter, Goblins may
        // still carry them so progression can never deadlock.
        return chapterPool;
    }


    if (enemyType == EnemyType::Orc)
    {
        // Orcs favor medium-sized vocabulary.
        for (
            const WordRecord& record
            :
            chapterPool
        )
        {
            if (
                record.length >= 4
                &&
                record.length <= 8
            )
            {
                preferred.push_back(
                    record
                );
            }
        }


        return
            preferred.empty()
            ?
            chapterPool
            :
            preferred;
    }


    // Beasts favor the harder/longer vocabulary.
    for (
        const WordRecord& record
        :
        chapterPool
    )
    {
        if (record.length >= 7)
        {
            preferred.push_back(
                record
            );
        }
    }


    return
        preferred.empty()
        ?
        chapterPool
        :
        preferred;
}


std::vector<WordRecord>
WordManager::GetWords(
    const std::vector<int>& excludedWordIds
) const
{
    const std::vector<WordRecord>
        chapterPool =
            BuildCurrentChapterPool(
                excludedWordIds
            );


    if (chapterPool.empty())
    {
        return {};
    }


    // Backward-compatible callers simply receive one random word
    // from the active chapter.
    const int index =
        GetRandomValue(
            0,
            static_cast<int>(
                chapterPool.size()
            )
            -
            1
        );


    return {
        chapterPool[index]
    };
}


std::vector<WordRecord>
WordManager::GetWords(
    const std::vector<int>& excludedWordIds,
    EnemyType enemyType
) const
{
    const std::vector<WordRecord>
        chapterPool =
            BuildCurrentChapterPool(
                excludedWordIds
            );


    if (chapterPool.empty())
    {
        return {};
    }


    const std::vector<WordRecord>
        priorityPool =
            BuildPriorityPool(
                chapterPool,
                enemyType
            );


    if (priorityPool.empty())
    {
        return {};
    }


    const int index =
        GetRandomValue(
            0,
            static_cast<int>(
                priorityPool.size()
            )
            -
            1
        );


    return {
        priorityPool[index]
    };
}


std::vector<WordRecord>
WordManager::GetWordsForChapter(
    const std::vector<int>& excludedWordIds,
    EnemyType enemyType,
    int chapter
) const
{
    if (chapter < 1)
    {
        return {};
    }


    std::vector<WordRecord>
        chapterPool;


    for (
        const WordRecord& record
        :
        words
    )
    {
        if (
            GameplayChapter(
                record.firstChapter
            )
            !=
            chapter
        )
        {
            continue;
        }


        if (
            ContainsId(
                excludedWordIds,
                record.id
            )
        )
        {
            continue;
        }


        chapterPool.push_back(
            record
        );
    }


    if (chapterPool.empty())
    {
        // Do NOT fall through into a later chapter here.
        // Empty means all remaining words in this chapter are
        // currently reserved on active creatures.
        return {};
    }


    const std::vector<WordRecord>
        priorityPool =
            BuildPriorityPool(
                chapterPool,
                enemyType
            );


    if (priorityPool.empty())
    {
        return {};
    }


    const int index =
        GetRandomValue(
            0,
            static_cast<int>(
                priorityPool.size()
            )
            -
            1
        );


    return {
        priorityPool[index]
    };
}


EnemyType WordManager::GetEnemyType(
    int recoveredCount
) const
{
    if (recoveredCount < 15)
    {
        return EnemyType::Goblin;
    }


    if (recoveredCount < 25)
    {
        return
            GetRandomValue(
                0,
                1
            )
            ==
            0
            ?
            EnemyType::Goblin
            :
            EnemyType::Orc;
    }


    const int roll =
        GetRandomValue(
            0,
            2
        );


    if (roll == 0)
    {
        return EnemyType::Goblin;
    }


    if (roll == 1)
    {
        return EnemyType::Orc;
    }


    return EnemyType::Beast;
}


bool WordManager::LoadWords(
    const std::string& filePath
)
{
    words.clear();


    std::ifstream file(
        filePath,
        std::ios::binary
    );


    if (!file)
    {
        return false;
    }


    std::ostringstream buffer;

    buffer
        <<
        file.rdbuf();


    const std::string text =
        buffer.str();


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


    std::size_t position =
        arrayStart + 1;


    while (
        position
        <
        text.size()
    )
    {
        const std::size_t objectStart =
            text.find(
                '{',
                position
            );


        const std::size_t arrayEnd =
            text.find(
                ']',
                position
            );


        if (
            arrayEnd
            !=
            std::string::npos
            &&
            (
                objectStart
                ==
                std::string::npos
                ||
                arrayEnd < objectStart
            )
        )
        {
            break;
        }


        if (
            objectStart
            ==
            std::string::npos
        )
        {
            break;
        }


        const std::size_t objectEnd =
            text.find(
                '}',
                objectStart
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


        WordRecord record;

        record.id =
            ParseIntField(
                objectText,
                "id"
            );


        record.word =
            ParseStringField(
                objectText,
                "word"
            );


        record.length =
            ParseIntField(
                objectText,
                "length"
            );


        record.firstChapter =
            ParseIntField(
                objectText,
                "first_chapter"
            );


        if (record.firstChapter < 0)
        {
            record.firstChapter = 1;
        }


        if (
            record.id >= 0
            &&
            !record.word.empty()
            &&
            record.length > 0
        )
        {
            words.push_back(
                record
            );
        }


        position =
            objectEnd + 1;
    }


    return !words.empty();
}


int WordManager::ParseIntField(
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

    bool foundDigit =
        false;


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
        foundDigit = true;

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


    return
        foundDigit
        ?
        value
        :
        -1;
}


std::string WordManager::ParseStringField(
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


    while (
        position
        <
        objectText.size()
    )
    {
        const char character =
            objectText[position];


        if (character == '"')
        {
            break;
        }


        if (
            character == '\\'
            &&
            position + 1
            <
            objectText.size()
        )
        {
            position++;


            const char escaped =
                objectText[position];


            switch (escaped)
            {
                case '"':
                    value += '"';
                    break;

                case '\\':
                    value += '\\';
                    break;

                case '/':
                    value += '/';
                    break;

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
                    value += escaped;
                    break;
            }
        }

        else
        {
            value += character;
        }


        position++;
    }


    return value;
}