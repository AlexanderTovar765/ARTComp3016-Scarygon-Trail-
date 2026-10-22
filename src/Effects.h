#pragma once
#include <string>

// This part of the code makes it how one Choice changes the party's four stats.
// So a positive = benefit, negative = suffer. Anything else left at 0 is unchanged.
struct Effects {
    int food   = 0;
    int health = 0;
    int sanity = 0;
    int wagon  = 0;

    // This part of the code is for who the health change hits: empty = every living member, or one member's name
    // (for example "Pa"). Food, sanity and wagon are then always shared.
    std::string member;

    bool isZero() const { return food == 0 && health == 0 && sanity == 0 && wagon == 0; }

    // This part of the code is for the readable summary such as "Food -5, Sanity +3" or "no change".
    std::string describe() const;
};
