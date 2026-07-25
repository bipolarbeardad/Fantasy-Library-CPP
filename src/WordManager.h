#pragma once

#include "Enemy.h"
#include <string>
#include <vector>

struct WordRecord {
    int id = -1;
    std::string word;
    int length = 0;
};

class WordManager {
public:
    explicit WordManager(const std::string& filePath = "words.json");

    bool IsLoaded() const;
    int GetTotalWordCount() const;

    std::vector<WordRecord> GetWords(
        const std::vector<int>& recoveredWordIds
    ) const;

    EnemyType GetEnemyType(int recoveredCount) const;

private:
    std::vector<WordRecord> words;
    bool loaded = false;

    bool LoadWords(const std::string& filePath);

    static int ParseIntField(
        const std::string& objectText,
        const std::string& fieldName
    );

    static std::string ParseStringField(
        const std::string& objectText,
        const std::string& fieldName
    );

    static bool ContainsId(
        const std::vector<int>& ids,
        int id
    );
};