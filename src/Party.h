#pragma once
#include <string>
#include <vector>

#include "Effects.h"
#include "PartyMember.h"
#include "PartyObserver.h"

// The wagon party: four PartyMembers (each with their own health) plus shared Food,
// Sanity and Wagon. Party is the subject of the Observer pattern: it notifies observers
// whenever a stat or a member's status changes.
//
// Rules:
//  - The party's Health stat is the average of the four members' health (the dead count as 0).
//  - A member dies at 0 health. Each death costs the survivors sanity (grief).
//  - The run ends when everyone is dead, or Sanity or the Wagon reaches 0.
//  - Food runs out -> every living member takes starvation damage each part.
class Party {
public:
    static constexpr int kMaxStat          = 100;
    static constexpr int kStartFood        = 60;

    // The trail's toll, taken after every event (one part of the day):
    static constexpr int kFoodPerPart      = 3;   // for a full party; fewer mouths eat less
    static constexpr int kSanityPerPart    = 4;   // the trail wears on everyone
    static constexpr int kWagonPerPart     = 2;   // wear and tear
    static constexpr int kHardshipPerPart  = 3;   // health every living member loses (frail members lose more)
    static constexpr int kStarvationDamage = 10;  // health lost per living member while food is 0
    static constexpr int kGriefSanity      = 8;   // sanity lost per member who dies

    Party();

    // Copies copy the stats and members but NOT the observers. (A copy is a "what if"
    // scratch party, e.g. for the simulator, and must not trigger the real game's observers.)
    Party(const Party& other);
    Party& operator=(const Party& other);  // silent: copies stats, keeps this party's observers

    void reset();

    // Applies a Choice's effects, keeping every stat within 0..kMaxStat.
    void apply(const Effects& effects);

    // End-of-part upkeep: the party eats, the trail wears on sanity and the wagon, and
    // everyone starves if the food has run out. Returns the change that actually happened
    // so it can be shown to the player.
    Effects advanceTime();

    bool isAlive() const;
    std::string causeOfDeath() const;  // empty while alive

    int getFood() const   { return m_food; }
    int getHealth() const;             // average of the members' health
    int getSanity() const { return m_sanity; }
    int getWagon() const  { return m_wagon; }

    const std::vector<PartyMember>& getMembers() const { return m_members; }
    int aliveCount() const;

    void addObserver(PartyObserver* observer);
    void removeObserver(PartyObserver* observer);

private:
    struct Snapshot {
        int food = 0;
        int health = 0;
        int sanity = 0;
        int wagon = 0;
        std::vector<PartyMember::Status> statuses;
    };

    Snapshot snapshot() const;
    void notifyChanges(const Snapshot& before) const;
    void changeMemberHealth(int delta, const std::string& memberName);
    void applyGrief(const Snapshot& before);
    int  foodToll() const;
    static int clampStat(int value);

    int m_food   = kStartFood;
    int m_sanity = kMaxStat;
    int m_wagon  = kMaxStat;
    std::vector<PartyMember> m_members;
    std::vector<PartyObserver*> m_observers;
};
