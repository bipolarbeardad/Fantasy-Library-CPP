#pragma once

#include <string>
#include <vector>


struct SaveData
{
    int wordsRecovered = 0;

    std::vector<int>
        recoveredWordIds;

    int displayIndex = 0;

    int musicVolume = 40;
};


class SaveManager
{
public:

    SaveManager(
        const std::string& saveFile = "save.json",
        const std::string& wordsFile = "words.json"
    );


    SaveData LoadSave() const;


    bool SaveGame(
        SaveData& data
    ) const;


private:

    std::string saveFile;
    std::string wordsFile;


    std::vector<int> ConvertOldWordsToIds(
        const std::vector<std::string>& oldWords
    ) const;


    static void RemoveDuplicateIds(
        std::vector<int>& ids
    );


    static std::string ReadFile(
        const std::string& filePath
    );


    static std::vector<int> ParseIntArray(
        const std::string& text,
        const std::string& fieldName
    );


    static std::vector<std::string> ParseStringArray(
        const std::string& text,
        const std::string& fieldName
    );


    static std::string ParseStringField(
        const std::string& objectText,
        const std::string& fieldName
    );


    static int ParseIntField(
        const std::string& objectText,
        const std::string& fieldName
    );
};