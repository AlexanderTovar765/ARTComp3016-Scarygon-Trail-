#include "Party.h"
#include <algorithm>

Party::Party() {
    reset();
}

Party::Party(const Party& other)
    : m_food(other.m_food), m_sanity(other.m_sanity), m_wagon(other.m_wagon),
      m_members(other.m_members) {}  // observers are deliberately not copied

Party& Party::operator=(const Party& other) {
    if (this != &other) {
        m_food = other.m_food;
        m_sanity = other.m_sanity;
        m_wagon = other.m_wagon;
        m_members = other.m_members;
    }
    return *this;
}

void Party::reset() {
    const Snapshot before = snapshot();

    m_food   = kStartFood;
    m_sanity = kMaxStat;
    m_wagon  = kMaxStat;
    m_members.clear();
    for (int i = 0; i < kMemberCount; ++i) {
        m_members.emplace_back(memberTemplates()[i].name, memberTemplates()[i].frailtyPercent);
    }

    notifyChanges(before);
}

int Party::clampStat(int value) {
    return std::max(0, std::min(value, static_cast<int>(kMaxStat)));
}

int Party::aliveCount() const {
    int alive = 0;
    for (const PartyMember& member : m_members) {
        if (member.isAlive()) ++alive;
    }
    return alive;
}

int Party::getHealth() const {
    if (m_members.empty()) return 0;
    int total = 0;
    for (const PartyMember& member : m_members) total += member.getHealth();
    return total / static_cast<int>(m_members.size());
}

int Party::foodToll() const {
    // Fewer mouths eat less, but never less than 1 each: 4 alive -> 3, 3 -> 3, 2 -> 2, 1 -> 1.
    const int size = static_cast<int>(m_members.size());
    if (size == 0) return 0;
    return (kFoodPerPart * aliveCount() + size - 1) / size;
}

void Party::changeMemberHealth(int delta, const std::string& memberName) {
    if (delta == 0) return;

    if (!memberName.empty()) {
        const int index = findMemberIndex(memberName);
        if (index >= 0 && index < static_cast<int>(m_members.size())) {
            m_members[static_cast<std::size_t>(index)].changeHealth(delta);
        }
        return;  // an unknown name changes nothing (the data loader rejects unknown names)
    }
    for (PartyMember& member : m_members) member.changeHealth(delta);
}

void Party::applyGrief(const Snapshot& before) {
    int newlyDead = 0;
    for (std::size_t i = 0; i < m_members.size() && i < before.statuses.size(); ++i) {
        if (before.statuses[i] != PartyMember::Status::Dead &&
            m_members[i].getStatus() == PartyMember::Status::Dead) {
            ++newlyDead;
        }
    }
    if (newlyDead > 0) m_sanity = clampStat(m_sanity - newlyDead * kGriefSanity);
}

void Party::apply(const Effects& effects) {
    const Snapshot before = snapshot();

    m_food   = clampStat(m_food   + effects.food);
    m_sanity = clampStat(m_sanity + effects.sanity);
    m_wagon  = clampStat(m_wagon  + effects.wagon);
    changeMemberHealth(effects.health, effects.member);
    applyGrief(before);

    notifyChanges(before);
}

Effects Party::advanceTime() {
    const Snapshot before = snapshot();

    m_food   = clampStat(m_food   - foodToll());
    m_sanity = clampStat(m_sanity - kSanityPerPart);
    m_wagon  = clampStat(m_wagon  - kWagonPerPart);
    for (PartyMember& member : m_members) member.changeHealth(-kHardshipPerPart);  // the trail is hard on everyone
    if (m_food == 0) {
        for (PartyMember& member : m_members) member.changeHealth(-kStarvationDamage);
    }
    applyGrief(before);

    notifyChanges(before);

    Effects change;  // what actually changed, for display
    change.food   = m_food   - before.food;
    change.health = getHealth() - before.health;
    change.sanity = m_sanity - before.sanity;
    change.wagon  = m_wagon  - before.wagon;
    return change;
}

bool Party::isAlive() const {
    return aliveCount() > 0 && m_sanity > 0 && m_wagon > 0;
}

std::string Party::causeOfDeath() const {
    if (aliveCount() <= 0) return "Everyone has died. The oxen continue without you, out of habit.";
    if (m_sanity <= 0)     return "Your sanity has left the wagon. It did not say where it was going.";
    if (m_wagon  <= 0)     return "The wagon has ceased to be a wagon and is now a pile of opinions about wagons.";
    return "";
}

// ---- Observer pattern: subject side -----------------------------------------

void Party::addObserver(PartyObserver* observer) {
    if (observer && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end()) {
        m_observers.push_back(observer);
    }
}

void Party::removeObserver(PartyObserver* observer) {
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
}

Party::Snapshot Party::snapshot() const {
    Snapshot s;
    s.food = m_food;
    s.health = getHealth();
    s.sanity = m_sanity;
    s.wagon = m_wagon;
    for (const PartyMember& member : m_members) s.statuses.push_back(member.getStatus());
    return s;
}

void Party::notifyChanges(const Snapshot& before) const {
    if (m_observers.empty()) return;

    const Snapshot now = snapshot();
    auto tell = [&](Stat stat, int oldValue, int newValue) {
        if (oldValue == newValue) return;
        for (PartyObserver* observer : m_observers) observer->onStatChanged(stat, oldValue, newValue);
    };
    tell(Stat::Food,   before.food,   now.food);
    tell(Stat::Health, before.health, now.health);
    tell(Stat::Sanity, before.sanity, now.sanity);
    tell(Stat::Wagon,  before.wagon,  now.wagon);

    if (before.statuses.size() == m_members.size()) {
        for (std::size_t i = 0; i < m_members.size(); ++i) {
            if (before.statuses[i] == now.statuses[i]) continue;
            for (PartyObserver* observer : m_observers) {
                observer->onMemberStatusChanged(m_members[i], before.statuses[i], now.statuses[i]);
            }
        }
    }
}
