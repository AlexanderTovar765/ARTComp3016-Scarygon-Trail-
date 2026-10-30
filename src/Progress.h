#pragma once
#include <string>
#include <vector>

#include "Animations.h"
#include "Difficulty.h"

// This part of the code is for the ways a journey can end. Finding them all is one reason to play again.
enum class EndingId {
    Intact = 0,    // everyone made it
    LostOne,       // three of four made it
    Half,          // two of four
    Lone,          // one survivor
    EveryoneDied,
    SanityLost,
    WagonLost,
    Count
};
constexpr int kEndingCount = static_cast<int>(EndingId::Count);

const char* endingName(EndingId id);    // This part of the code is if short name used in the save file, e.g. "lost_one"
const char* endingTitle(EndingId id);   // This part of the code is what the player sees, e.g. "A small loss"

// This part of the code is for what the player has discovered across runs, saved to a small text file (save/progress.txt):
//  - endings reached, the scenes and reaction animations seen, which difficulties were cleared,
//  - and the last difficulty chosen.
// A missing or damaged file just means starting fresh; a file that can't be written is not an error
// (progress then lasts for this session only).
class Progress {
public:
    Difficulty difficulty = Difficulty::Normal;
    int runs = 0;                                   // journey begins

    // This part of the code is checks which ending a finished journey counts as.
    static EndingId classify(bool survived, int aliveCount, int sanity, int wagon);

    static Progress load(const std::string& path, std::vector<std::string>* warnings = nullptr);
    bool save(const std::string& path) const;       // This part of the code activates false if the file could not be written

    // This part of the code is for each returns true the first time (a new discovery).
    bool markScene(SceneId id);
    bool markAction(ActionId id);
    bool markEnding(EndingId id);
    void markCleared(Difficulty d);

    bool sceneSeen(SceneId id) const;
    bool actionSeen(ActionId id) const;
    bool endingSeen(EndingId id) const;
    bool cleared(Difficulty d) const;

    int scenesSeen() const;
    int actionsSeen() const;
    int endingsSeen() const;
    int clearedCount() const;
    int animationsSeen() const { return scenesSeen() + actionsSeen(); }
    static constexpr int animationTotal() { return kSceneCount + kActionCount; }

private:
    bool m_scenes[kSceneCount] = {};
    bool m_actions[kActionCount + 1] = {};
    bool m_endings[kEndingCount] = {};
    bool m_cleared[3] = {};
};
