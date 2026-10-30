#include "ThresholdWatcher.h"

void ThresholdWatcher::onStatChanged(Stat stat, int oldValue, int newValue) {
    switch (stat) {
    case Stat::Food:
        check(Condition::LowFood, kLowFood, oldValue, newValue, "Warning: the food is running low.");
        break;
    case Stat::Wagon:
        check(Condition::LowWagon, kLowWagon, oldValue, newValue, "Warning: the wagon is making new and worrying noises.");
        break;
    case Stat::Health:
        check(Condition::LowHealth, kLowHealth, oldValue, newValue, "Warning: the family is in poor health.");
        break;
    case Stat::Sanity:
        break;  //This part of the code makes sure the sanity is handled by the NarrativeState instead
    }
}

void ThresholdWatcher::check(Condition condition, int threshold, int oldValue, int newValue,
                             const char* warning) {
    const bool isLow  = newValue <= threshold;
    const bool wasLow = oldValue <= threshold;

    m_active.set(condition, isLow);
    if (isLow && !wasLow) {          // This part of the code makes sure only a drop INTO the danger zone warns, never a recovery
        m_warnings.push_back(warning);
    }
}

std::vector<std::string> ThresholdWatcher::takeWarnings() {
    std::vector<std::string> out;
    out.swap(m_warnings);
    return out;
}
