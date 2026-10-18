#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Conditions.h"
#include "Event.h"
#include "TimeOfDay.h"

// Holds every Event, keyed by (day, part of day), and tracks where the party is
// on the trail. Events are loaded from a text file (see data/events.txt for the format).
class Trail {
public:
    // Loads events from a file. Returns false if the file can't be opened or contains no
    // valid events. Malformed events are skipped (never a crash) and described in getWarnings().
    bool loadFromFile(const std::string& path);

    const std::vector<std::string>& getWarnings() const { return m_warnings; }
    std::size_t eventCount() const { return m_eventCount; }  // every event, including variants

    // The event for a slot. If a desperate variant's condition is in 'active' it wins over the
    // normal event; otherwise the normal one is returned. nullptr if the slot has nothing.
    const Event* getEvent(int day, TimePart part, ConditionSet active = ConditionSet()) const;
    const Event* currentEvent(ConditionSet active = ConditionSet()) const {
        return getEvent(m_day, m_part, active);
    }
    bool hasDay(int day) const;
    int lastDay() const;  // highest day that has an event (1 if there are none)

    int currentDay() const       { return m_day; }
    TimePart currentPart() const { return m_part; }
    bool isLastPartOfDay() const { return m_part == TimePart::Night; }

    void advance();  // next part of the day; after Night it rolls over to Dawn of the next day
    void reset();    // back to Day 1, Dawn

private:
    std::map<std::pair<int, int>, std::vector<Event>> m_events;  // key: (day, part index)
    std::size_t m_eventCount = 0;
    std::vector<std::string> m_warnings;
    int m_day = 1;
    TimePart m_part = TimePart::Dawn;
};
