#include "SaveManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_map>


SaveManager::SaveManager(
    const std::string& saveFile,
    const std::string& wordsFile
)
    :
    saveFile(saveFile),
    wordsFile(wordsFile)
{
}


SaveData SaveManager::LoadSave() const
{
    SaveData data;


    const std::string text =
        ReadFile(
            saveFile
        );


    if (text.empty())
    {
        return data;
    }


    if (
        text.find(
            "\"recovered_word_ids\""
        )
        !=
        std::string::npos
    )
    {
        data.recoveredWordIds =
            ParseIntArray(
                text,
                "recovered_word_ids"
            );
    }

    else
    {
        data.recoveredWordIds =
            ConvertOldWordsToIds(
                ParseStringArray(
                    text,
                    "unique_words"
                )
            );
    }


    RemoveDuplicateIds(
        data.recoveredWordIds
    );


    data.wordsRecovered =
        static_cast<int>(
            data.recoveredWordIds.size()
        );


    const int savedDisplay =
        ParseIntField(
            text,
            "display_index"
        );


    if (savedDisplay >= 0)
    {
        data.displayIndex =
            savedDisplay;
    }


    const int savedVolume =
        ParseIntField(
            text,
            "music_volume"
        );


    if (savedVolume >= 0)
    {
        data.musicVolume =
            savedVolume;
    }


    return data;
}


bool SaveManager::SaveGame(
    SaveData& data
) const
{
    RemoveDuplicateIds(
        data.recoveredWordIds
    );


    data.wordsRecovered =
        static_cast<int>(
            data.recoveredWordIds.size()
        );


    std::ofstream file(
        saveFile,
        std::ios::binary
    );


    if (!file)
    {
        return false;
    }


    file
        << "{\n"
        << "  \"words_recovered\": "
        << data.wordsRecovered
        << ",\n"
        << "  \"recovered_word_ids\": [";


    for (
        std::size_t index = 0;
        index
        <
        data.recoveredWordIds.size();
        index++
    )
    {
        if (index > 0)
        {
            file << ", ";
        }


        file
            << data.recoveredWordIds[index];
    }


    file
        << "],\n"
        << "  \"settings\": {\n"
        << "    \"display_index\": "
        << data.displayIndex
        << ",\n"
        << "    \"music_volume\": "
        << data.musicVolume
        << "\n"
        << "  }\n"
        << "}\n";


    return true;
}


std::vector<int>
SaveManager::ConvertOldWordsToIds(
    const std::vector<std::string>& oldWords
) const
{
    if (oldWords.empty())
    {
        return {};
    }


    const std::string text =
        ReadFile(
            wordsFile
        );


    if (text.empty())
    {
        return {};
    }


    std::unordered_map<std::string, int>
        lookup;


    std::size_t position =
        text.find(
            "\"words\""
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return {};
    }


    position =
        text.find(
            '[',
            position
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return {};
    }


    position++;


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
                arrayEnd
                <
                objectStart
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


        const int id =
            ParseIntField(
                objectText,
                "id"
            );


        std::string word =
            ParseStringField(
                objectText,
                "word"
            );


        for (char& character : word)
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


        if (
            id >= 0
            &&
            !word.empty()
        )
        {
            lookup[word] =
                id;
        }


        position =
            objectEnd + 1;
    }


    std::vector<int> result;


    for (
        std::string word
        :
        oldWords
    )
    {
        for (char& character : word)
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


        const auto found =
            lookup.find(
                word
            );


        if (
            found
            !=
            lookup.end()
        )
        {
            result.push_back(
                found->second
            );
        }
    }


    RemoveDuplicateIds(
        result
    );


    return result;
}


void SaveManager::RemoveDuplicateIds(
    std::vector<int>& ids
)
{
    std::vector<int> unique;


    for (int id : ids)
    {
        if (
            std::find(
                unique.begin(),
                unique.end(),
                id
            )
            ==
            unique.end()
        )
        {
            unique.push_back(
                id
            );
        }
    }


    ids = std::move(
        unique
    );
}


std::string SaveManager::ReadFile(
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


std::vector<int>
SaveManager::ParseIntArray(
    const std::string& text,
    const std::string& fieldName
)
{
    std::vector<int> values;


    const std::string key =
        "\""
        +
        fieldName
        +
        "\"";


    std::size_t position =
        text.find(
            key
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return values;
    }


    const std::size_t start =
        text.find(
            '[',
            position
        );


    const std::size_t end =
        text.find(
            ']',
            start
        );


    if (
        start
        ==
        std::string::npos
        ||
        end
        ==
        std::string::npos
    )
    {
        return values;
    }


    position =
        start + 1;


    while (position < end)
    {
        while (
            position < end
            &&
            !std::isdigit(
                static_cast<unsigned char>(
                    text[position]
                )
            )
        )
        {
            position++;
        }


        if (position >= end)
        {
            break;
        }


        int value = 0;


        while (
            position < end
            &&
            std::isdigit(
                static_cast<unsigned char>(
                    text[position]
                )
            )
        )
        {
            value =
                value * 10
                +
                (
                    text[position]
                    -
                    '0'
                );


            position++;
        }


        values.push_back(
            value
        );
    }


    return values;
}


std::vector<std::string>
SaveManager::ParseStringArray(
    const std::string& text,
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


    std::size_t position =
        text.find(
            key
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return values;
    }


    const std::size_t start =
        text.find(
            '[',
            position
        );


    const std::size_t end =
        text.find(
            ']',
            start
        );


    if (
        start
        ==
        std::string::npos
        ||
        end
        ==
        std::string::npos
    )
    {
        return values;
    }


    position =
        start + 1;


    while (position < end)
    {
        const std::size_t quoteStart =
            text.find(
                '"',
                position
            );


        if (
            quoteStart
            ==
            std::string::npos
            ||
            quoteStart >= end
        )
        {
            break;
        }


        const std::size_t quoteEnd =
            text.find(
                '"',
                quoteStart + 1
            );


        if (
            quoteEnd
            ==
            std::string::npos
            ||
            quoteEnd > end
        )
        {
            break;
        }


        values.push_back(
            text.substr(
                quoteStart + 1,
                quoteEnd
                -
                quoteStart
                -
                1
            )
        );


        position =
            quoteEnd + 1;
    }


    return values;
}


std::string SaveManager::ParseStringField(
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
            position
        );


    position =
        objectText.find(
            '"',
            position
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        return "";
    }


    const std::size_t end =
        objectText.find(
            '"',
            position + 1
        );


    if (
        end
        ==
        std::string::npos
    )
    {
        return "";
    }


    return objectText.substr(
        position + 1,
        end - position - 1
    );
}


int SaveManager::ParseIntField(
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
            position
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