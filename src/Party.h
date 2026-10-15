#pragma once
#include <string>
#include "Effects.h"

// The player's wagon party. For now the stats are shared by everyone in it.
// Every stat runs from 0 to kMaxStat. Health, sanity or wagon reaching 0 ends the run;
// food reaching 0 means starvation, which drains health every part of the day.
class Party {
public:
    static constexpr int kMaxStat          = 100;
    static constexpr int kStartFood        = 60;
    // The trail's toll, taken after every event (one part of the day):
    static constexpr int kFoodPerPart      = 3;   // the party eats
    static constexpr int kSanityPerPart    = 4;   // the trail wears on everyone
    static constexpr int kWagonPerPart     = 2;   // wear and tear
    static constexpr int kStarvationDamage = 10;  // health lost per part while food is 0

    Party() { reset(); }

    void reset();

    // Applies a Choice's effects, keeping every stat within 0..kMaxStat.
    void apply(const Effects& effects);

    // End-of-part upkeep: the party eats, the trail wears on sanity and the wagon, and the
    // party starves if the food has run out. Returns the change that actually happened
    // so it can be shown to the player.
    Effects advanceTime();

    bool isAlive() const;
    std::string causeOfDeath() const;  // empty while alive

    int getFood() const   { return m_food; }
    int getHealth() const { return m_health; }
    int getSanity() const { return m_sanity; }
    int getWagon() const  { return m_wagon; }

private:
    static int clampStat(int value);

    int m_food   = kStartFood;
    int m_health = kMaxStat;
    int m_sanity = kMaxStat;
    int m_wagon  = kMaxStat;
};
