#pragma once

#include "Enemy.h"

#include <string>
#include <vector>


struct WordRecord
{
    int id = -1;
    std::string word;
    int length = 0;

    // The first story chapter in which this unique word occurs.
    // words.json uses 0 for title/front-matter words; those are
    // treated as part of Chapter 1 for gameplay progression.
    int firstChapter = 1;
};


class WordManager
{
public:
    explicit WordManager(
        const std::string& filePath = "words.json"
    );

    bool IsLoaded() const;
    int GetTotalWordCount() const;


    // Backward-compatible version. Uses general chapter-priority
    // selection when the caller does not specify an enemy type.
    std::vector<WordRecord> GetWords(
        const std::vector<int>& excludedWordIds
    ) const;


    // Chapter-priority + enemy-difficulty selection.
    std::vector<WordRecord> GetWords(
        const std::vector<int>& excludedWordIds,
        EnemyType enemyType
    ) const;


    // Strict chapter selection. The chapter is calculated from
    // recovered IDs only; reserved words cannot advance progression.
    std::vector<WordRecord> GetWordsForChapter(
        const std::vector<int>& excludedWordIds,
        EnemyType enemyType,
        int chapter
    ) const;


    // Earliest chapter that still contains an unrecovered word.
    // Returns -1 once every word has been recovered.
    int GetCurrentChapter(
        const std::vector<int>& recoveredWordIds
    ) const;


    // True when no unique word whose first appearance belongs to
    // this chapter remains unrecovered.
    bool IsChapterComplete(
        int chapter,
        const std::vector<int>& recoveredWordIds
    ) const;


    // Chooses which creature appears based on overall restoration
    // progress. New creature types are introduced gradually.
    EnemyType GetEnemyType(
        int recoveredCount
    ) const;


private:
    std::vector<WordRecord> words;
    bool loaded = false;

    bool LoadWords(
        const std::string& filePath
    );

    std::vector<WordRecord> BuildCurrentChapterPool(
        const std::vector<int>& excludedWordIds
    ) const;

    std::vector<WordRecord> BuildPriorityPool(
        const std::vector<WordRecord>& chapterPool,
        EnemyType enemyType
    ) const;

    static bool IsCommonWord(
        const std::string& word
    );

    static int GameplayChapter(
        int firstChapter
    );

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