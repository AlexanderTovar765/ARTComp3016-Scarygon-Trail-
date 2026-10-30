#pragma once
#include <string>
#include <vector>

#include "PartyObserver.h"

// This part of the code is the observer: turns changes in a member's status into short messages ("Granny has fallen ill.",
// "Pa has died. ...") that the game shows after the choice that caused them.
class MemberWatcher : public PartyObserver {
public:
    void onStatChanged(Stat, int, int) override {}  // This part of the code checks only members matter here

    void onMemberStatusChanged(const PartyMember& member,
                               PartyMember::Status oldStatus,
                               PartyMember::Status newStatus) override;

    std::vector<std::string> takeMessages();  // This part of the code returns the pending messages and clears them
    void clearMessages() { m_messages.clear(); }

    // This part of the code checks true if any member has died since the messages were last taken
    bool sawDeath() const { return m_sawDeath; }

private:
    std::vector<std::string> m_messages;
    bool m_sawDeath = false;
};
