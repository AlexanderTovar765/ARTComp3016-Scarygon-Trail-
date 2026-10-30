#include "Difficulty.h"

#include <cctype>

namespace {
    // This part of the code checks if the order matches the enum: Easy, Normal, Hard.
    // Its tuned with tools/balance_sim.cpp (see docs/balance-report.md).
    //                                    food  eat san wag hard starve harm help
    const DifficultyRules kRules[3] = {
        /* Easy   */                    {  70,   3,  3,  2,   3,   10,  100, 100 },
        /* Normal */                    {  60,   3,  4,  2,   3,   10,  115, 100 },
        /* Hard   */                    {  50,   4,  5,  2,   3,   11,  125, 100 },
    };
}

const DifficultyRules& rulesFor(Difficulty difficulty) {
    const int index = static_cast<int>(difficulty);
    return kRules[(index >= 0 && index < 3) ? index : 1];
}

const char* difficultyName(Difficulty difficulty) {
    switch (difficulty) {
    case Difficulty::Easy:   return "Easy";
    case Difficulty::Normal: return "Normal";
    case Difficulty::Hard:   return "Hard";
    }
    return "Normal";
}

Difficulty nextDifficulty(Difficulty difficulty) {
    switch (difficulty) {
    case Difficulty::Easy:   return Difficulty::Normal;
    case Difficulty::Normal: return Difficulty::Hard;
    case Difficulty::Hard:   break;
    }
    return Difficulty::Easy;
}

bool parseDifficulty(const std::string& text, Difficulty& out) {
    std::string v;
    for (char c : text) v += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (v == "easy")   { out = Difficulty::Easy;   return true; }
    if (v == "normal") { out = Difficulty::Normal; return true; }
    if (v == "hard")   { out = Difficulty::Hard;   return true; }
    return false;
}
