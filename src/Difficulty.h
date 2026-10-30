#pragma once
#include <string>

// This part of the code sets the three difficulty levels. The rules differ only in numbers (how hard the trail's toll is and
// how much choices help or hurt); the story, the events and the narrator's lying are all the same.
enum class Difficulty { Easy, Normal, Hard };

struct DifficultyRules {
    int startFood;          // This is for food at the start of a journey
    int foodPerPart;        // This is for food eaten per part of the day by a full party
    int sanityPerPart;      // This is for sanity the trail wears away each part
    int wagonPerPart;       // This is for wear and tear on the wagon each part
    int hardshipPerPart;    // This is for health every living member loses each part (frail members lose more)
    int starvationDamage;   // This is for health each living member loses per part while food is 0
    int harmPercent;        // This is for scales the NEGATIVE effects of a choice (100 = as written in events.txt)
    int helpPercent;        // This is for scales the POSITIVE effects of a choice
};

const DifficultyRules& rulesFor(Difficulty difficulty);

const char* difficultyName(Difficulty difficulty);                  // "Easy", "Normal", "Hard"
Difficulty  nextDifficulty(Difficulty difficulty);                  // Easy -> Normal -> Hard -> Easy
bool        parseDifficulty(const std::string& text, Difficulty& out);   
