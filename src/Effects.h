#pragma once
#include <string>

// How one Choice changes the party's four stats.
// Positive = benefit, negative = suffer. Anything left at 0 is unchanged.
struct Effects {
    int food   = 0;
    int health = 0;
    int sanity = 0;
    int wagon  = 0;

    // Who the health change hits: empty = every living member, or one member's name
    // (canonical, e.g. "Pa"). Food, sanity and wagon are always shared.
    std::string member;

    bool isZero() const { return food == 0 && health == 0 && sanity == 0 && wagon == 0; }

    // Readable summary such as "Food -5, Sanity +3" (or "no change").
    std::string describe() const;
};
