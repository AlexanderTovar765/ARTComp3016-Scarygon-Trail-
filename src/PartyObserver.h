#pragma once
#include "PartyMember.h"
#include "Stat.h"

// This part of the code is the OBSERVER PATTERN. Party is the "subject": whenever one of its stats or a member's
// status changes, it tells every registered observer. Party knows nothing about what the
// observers do with the news, which keeps it decoupled from the UI, audio and event logic.
// Current observers: ThresholdWatcher (low-stat warnings and desperate events) and
// MemberWatcher (illness and death messages).
class PartyObserver {
public:
    virtual ~PartyObserver() = default;

    // This part of the code checks a stat changed. For Health, the values are the party average.
    virtual void onStatChanged(Stat stat, int oldValue, int newValue) = 0;

    // This part of the code checks a member's status changed (fell ill, recovered, died). Optional to implement.
    virtual void onMemberStatusChanged(const PartyMember& /*member*/,
                                       PartyMember::Status /*oldStatus*/,
                                       PartyMember::Status /*newStatus*/) {}
};
