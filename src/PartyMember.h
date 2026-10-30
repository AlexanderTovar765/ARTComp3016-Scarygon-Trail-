#pragma once
#include <string>

// This part of the code checks one member of the wagon party, with their own health.
// The party composes four of these (see Party). A member is Healthy, Ill (health below
// kIllBelow) or Dead (health 0, permanent). Frailty scales the damage a member takes:
// 100 = normal, 150 = takes 50% more damage, 90 = a bit tougher than average.
class PartyMember {
public:
    enum class Status { Healthy, Ill, Dead };

    static constexpr int kMaxHealth = 100;
    static constexpr int kIllBelow  = 40;

    PartyMember(std::string name, int frailtyPercent);

    void reset();

    // This part of the code checks negative values are damage (scaled by frailty, rounded up); positive values heal.
    // A dead member is unaffected.
    void changeHealth(int delta);

    const std::string& getName() const { return m_name; }
    int  getHealth() const             { return m_health; }
    int  getFrailtyPercent() const     { return m_frailty; }
    bool isAlive() const               { return m_health > 0; }
    Status getStatus() const;

private:
    std::string m_name;
    int m_frailty;
    int m_health = kMaxHealth;
};

// This part of the code is for The starting roster. Order matters: it is the order they are drawn on screen.
constexpr int kMemberCount = 4;

struct MemberTemplate {
    const char* name;
    int frailtyPercent;
};

const MemberTemplate* memberTemplates();

// Index of the member with this name (ignores case), or -1 if there is no such member.
int findMemberIndex(const std::string& name);
