#include "Party.h"
#include <algorithm>

void Party::reset() {
    m_food   = kStartFood;
    m_health = kMaxStat;
    m_sanity = kMaxStat;
    m_wagon  = kMaxStat;
}

int Party::clampStat(int value) {
    return std::max(0, std::min(value, static_cast<int>(kMaxStat)));
}

void Party::apply(const Effects& effects) {
    m_food   = clampStat(m_food   + effects.food);
    m_health = clampStat(m_health + effects.health);
    m_sanity = clampStat(m_sanity + effects.sanity);
    m_wagon  = clampStat(m_wagon  + effects.wagon);
}

Effects Party::advanceTime() {
    Effects change;

    const int foodBefore = m_food;
    const int sanityBefore = m_sanity;
    const int wagonBefore = m_wagon;
    m_food   = clampStat(m_food   - kFoodPerPart);
    m_sanity = clampStat(m_sanity - kSanityPerPart);
    m_wagon  = clampStat(m_wagon  - kWagonPerPart);
    change.food   = m_food   - foodBefore;    // the amounts that actually changed
    change.sanity = m_sanity - sanityBefore;
    change.wagon  = m_wagon  - wagonBefore;

    if (m_food == 0) {
        const int healthBefore = m_health;
        m_health = clampStat(m_health - kStarvationDamage);
        change.health = m_health - healthBefore;
    }
    return change;
}

bool Party::isAlive() const {
    return m_health > 0 && m_sanity > 0 && m_wagon > 0;
}

std::string Party::causeOfDeath() const {
    if (m_health <= 0) return "Everyone has died. The oxen continue without you, out of habit.";
    if (m_sanity <= 0) return "Your sanity has left the wagon. It did not say where it was going.";
    if (m_wagon  <= 0) return "The wagon has ceased to be a wagon and is now a pile of opinions about wagons.";
    return "";
}
