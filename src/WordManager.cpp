#include "WordManager.h"
#include "raylib.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

WordManager::WordManager(const std::string& filePath) {
    loaded = LoadWords(filePath);
}

bool WordManager::IsLoaded() const {
    return loaded;
}

int WordManager::GetTotalWordCount() const {
    return static_cast<int>(words.size());
}

bool WordManager::ContainsId(
    const std::vector<int>& ids,
    int id
) {
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

std::vector<WordRecord> WordManager::GetWords(
    const std::vector<int>& recoveredWordIds
) const {
    std::vector<WordRecord> availableWords;

    for (const WordRecord& record : words) {
        if (!ContainsId(recoveredWordIds, record.id)) {
            availableWords.push_back(record);
        }
    }

    if (availableWords.empty()) {
        return {};
    }

    const int recoveredCount =
        static_cast<int>(recoveredWordIds.size());

    std::vector<WordRecord> pool;

    // First 15: prefer exactly 3-letter words.
    if (recoveredCount < 15) {
        for (const WordRecord& record : availableWords) {
            if (record.length == 3) {
                pool.push_back(record);
            }
        }
    }
    // Words 15-24: only 4 letters or shorter.
    else if (recoveredCount < 25) {
        for (const WordRecord& record : availableWords) {
            if (record.length <= 4) {
                pool.push_back(record);
            }
        }
    }
    // Word 25+: full vocabulary.
    else {
        pool = availableWords;
    }

    // Safety fallback.
    if (pool.empty()) {
        if (recoveredCount < 25) {
            int shortestLength = availableWords.front().length;

            for (const WordRecord& record : availableWords) {
                shortestLength = std::min(shortestLength, record.length);
            }

            for (const WordRecord& record : availableWords) {
                if (record.length == shortestLength) {
                    pool.push_back(record);
                }
            }
        } else {
            pool = availableWords;
        }
    }

    int count = 1;

    // Preserve current Python behavior: after 25 words,
    // a 20% chance of requesting a two-word enemy.
    if (recoveredCount >= 25 && GetRandomValue(1, 5) == 5) {
        count = 2;
    }

    count = std::min(count, static_cast<int>(pool.size()));

    std::vector<WordRecord> result;
    result.reserve(count);

    // Random sample without duplicates.
    for (int i = 0; i < count; ++i) {
        const int index = GetRandomValue(
            0,
            static_cast<int>(pool.size()) - 1
        );

        result.push_back(pool[index]);
        pool.erase(pool.begin() + index);
    }

    return result;
}

EnemyType WordManager::GetEnemyType(int recoveredCount) const {
    if (recoveredCount < 15) {
        return EnemyType::Goblin;
    }

    if (recoveredCount < 25) {
        return GetRandomValue(0, 1) == 0
            ? EnemyType::Goblin
            : EnemyType::Orc;
    }

    const int roll = GetRandomValue(0, 2);

    if (roll == 0) return EnemyType::Goblin;
    if (roll == 1) return EnemyType::Orc;
    return EnemyType::Beast;
}

bool WordManager::LoadWords(const std::string& filePath) {
    words.clear();

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    const std::string text = buffer.str();

    const std::size_t wordsKey = text.find("\"words\"");
    if (wordsKey == std::string::npos) {
        return false;
    }

    const std::size_t arrayStart = text.find('[', wordsKey);
    if (arrayStart == std::string::npos) {
        return false;
    }

    std::size_t position = arrayStart + 1;

    while (position < text.size()) {
        const std::size_t objectStart = text.find('{', position);
        const std::size_t arrayEnd = text.find(']', position);

        if (
            arrayEnd != std::string::npos &&
            (objectStart == std::string::npos || arrayEnd < objectStart)
        ) {
            break;
        }

        if (objectStart == std::string::npos) {
            break;
        }

        const std::size_t objectEnd = text.find('}', objectStart);
        if (objectEnd == std::string::npos) {
            break;
        }

        const std::string objectText = text.substr(
            objectStart,
            objectEnd - objectStart + 1
        );

        WordRecord record;
        record.id = ParseIntField(objectText, "id");
        record.word = ParseStringField(objectText, "word");
        record.length = ParseIntField(objectText, "length");

        if (record.id >= 0 && !record.word.empty() && record.length > 0) {
            words.push_back(record);
        }

        position = objectEnd + 1;
    }

    return !words.empty();
}

int WordManager::ParseIntField(
    const std::string& objectText,
    const std::string& fieldName
) {
    const std::string key = "\"" + fieldName + "\"";

    std::size_t position = objectText.find(key);
    if (position == std::string::npos) return -1;

    position = objectText.find(':', position + key.size());
    if (position == std::string::npos) return -1;

    ++position;

    while (
        position < objectText.size() &&
        std::isspace(static_cast<unsigned char>(objectText[position]))
    ) {
        ++position;
    }

    int value = 0;
    bool foundDigit = false;

    while (
        position < objectText.size() &&
        std::isdigit(static_cast<unsigned char>(objectText[position]))
    ) {
        foundDigit = true;
        value = value * 10 + (objectText[position] - '0');
        ++position;
    }

    return foundDigit ? value : -1;
}

std::string WordManager::ParseStringField(
    const std::string& objectText,
    const std::string& fieldName
) {
    const std::string key = "\"" + fieldName + "\"";

    std::size_t position = objectText.find(key);
    if (position == std::string::npos) return "";

    position = objectText.find(':', position + key.size());
    if (position == std::string::npos) return "";

    position = objectText.find('"', position + 1);
    if (position == std::string::npos) return "";

    ++position;
    std::string value;

    while (position < objectText.size()) {
        const char character = objectText[position];

        if (character == '"') {
            break;
        }

        if (character == '\\' && position + 1 < objectText.size()) {
            ++position;
            const char escaped = objectText[position];

            switch (escaped) {
                case '"': value += '"'; break;
                case '\\': value += '\\'; break;
                case '/': value += '/'; break;
                case 'n': value += '\n'; break;
                case 'r': value += '\r'; break;
                case 't': value += '\t'; break;
                default: value += escaped; break;
            }
        } else {
            value += character;
        }

        ++position;
    }

    return value;
}