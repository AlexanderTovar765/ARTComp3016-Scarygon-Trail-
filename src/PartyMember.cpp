#include "PartyMember.h"

#include <algorithm>
#include <cctype>

PartyMember::PartyMember(std::string name, int frailtyPercent)
    : m_name(std::move(name)), m_frailty(std::max(1, frailtyPercent)) {}

void PartyMember::reset() {
    m_health = kMaxHealth;
}

void PartyMember::changeHealth(int delta) {
    if (!isAlive()) return;  // the dead stay dead

    if (delta < 0) {
        const int damage = ((-delta) * m_frailty + 99) / 100;  // frailty-scaled, rounded up
        m_health -= damage;
    } else {
        m_health += delta;
    }
    m_health = std::max(0, std::min(m_health, kMaxHealth));
}

PartyMember::Status PartyMember::getStatus() const {
    if (m_health <= 0)         return Status::Dead;
    if (m_health < kIllBelow)  return Status::Ill;
    return Status::Healthy;
}

const MemberTemplate* memberTemplates() {
    static const MemberTemplate templates[kMemberCount] = {
        { "Ma",     100 },
        { "Pa",      90 },
        { "Junior", 120 },
        { "Granny", 150 },
    };
    return templates;
}

int findMemberIndex(const std::string& name) {
    auto lower = [](std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    };
    const std::string wanted = lower(name);
    for (int i = 0; i < kMemberCount; ++i) {
        if (wanted == lower(memberTemplates()[i].name)) return i;
    }
    return -1;
}
