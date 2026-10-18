#include "MemberWatcher.h"

namespace {
    std::string deathMessage(const std::string& name) {
        if (name == "Ma")     return "Ma has died. She leaves behind strong opinions about the wagon.";
        if (name == "Pa")     return "Pa has died. He would like it noted that he was right.";
        if (name == "Junior") return "Junior has died. The curse, it turns out, was real.";
        if (name == "Granny") return "Granny has died. She looks well, considering.";
        return name + " has died.";
    }
}

void MemberWatcher::onMemberStatusChanged(const PartyMember& member,
                                          PartyMember::Status oldStatus,
                                          PartyMember::Status newStatus) {
    using Status = PartyMember::Status;
    if (newStatus == Status::Dead) {
        m_messages.push_back(deathMessage(member.getName()));
        m_sawDeath = true;
    } else if (newStatus == Status::Ill && oldStatus == Status::Healthy) {
        m_messages.push_back(member.getName() + " has fallen ill.");
    } else if (newStatus == Status::Healthy && oldStatus == Status::Ill) {
        m_messages.push_back(member.getName() + " is feeling better.");
    }
}

std::vector<std::string> MemberWatcher::takeMessages() {
    std::vector<std::string> out;
    out.swap(m_messages);
    m_sawDeath = false;
    return out;
}
