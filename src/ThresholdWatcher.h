#pragma once
#include <string>
#include <vector>

#include "Conditions.h"
#include "PartyObserver.h"

// Observer: watches the party's stats and keeps track of which are dangerously low.
//  - active() says which conditions are true right now; the Trail uses that to swap a
//    normal event for its "desperate" variant (e.g. a LowFood version of the noon event).
//  - takeWarnings() hands over the one-off "Warning: ..." lines for stats that have just
//    dropped into the danger zone, so the game can show them once.
class ThresholdWatcher : public PartyObserver {
public:
    // A stat at or below its threshold counts as low.
    static constexpr int kLowFood   = 25;
    static constexpr int kLowWagon  = 75;
    static constexpr int kLowHealth = 40;

    void onStatChanged(Stat stat, int oldValue, int newValue) override;

    ConditionSet active() const { return m_active; }

    std::vector<std::string> takeWarnings();  // returns the pending warnings and clears them
    void clearWarnings() { m_warnings.clear(); }

private:
    void check(Condition condition, int threshold, int oldValue, int newValue, const char* warning);

    ConditionSet m_active;
    std::vector<std::string> m_warnings;
};
