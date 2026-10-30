#include "Trail.h"

#include "PartyMember.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <optional>

namespace {
    // This part of the code is for the soft limits so text fits the panel and buttons. Going over only produces a warning
    // (the game still runs; the text just gets cut off with "...").
    constexpr std::size_t kMaxChoices      = 4;
    constexpr std::size_t kMaxLabelChars   = 46;
    constexpr std::size_t kMaxTextChars    = 200;
    constexpr std::size_t kMaxResultChars  = 260;
    constexpr int         kMaxEffectAmount = 100;

    std::string trim(const std::string& s) {
        std::size_t start = 0;
        std::size_t end = s.size();
        while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        return s.substr(start, end - start);
    }

    std::string toLower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    // This part of the code is for the built-in font only draws plain ASCII. Convert the typographic characters word
    // processors like to insert (curly quotes, dashes, ellipsis) and replace anything else.
    std::string toAscii(const std::string& in) {
        std::string out;
        out.reserve(in.size());
        std::size_t i = 0;
        while (i < in.size()) {
            const unsigned char c = static_cast<unsigned char>(in[i]);
            if (c < 0x80) {
                out += (c == '\t') ? ' ' : static_cast<char>(c);
                ++i;
            } else if (c == 0xE2 && i + 2 < in.size() && static_cast<unsigned char>(in[i + 1]) == 0x80) {
                const unsigned char t = static_cast<unsigned char>(in[i + 2]);
                if (t == 0x98 || t == 0x99)      out += '\'';
                else if (t == 0x9C || t == 0x9D) out += '"';
                else if (t == 0x93 || t == 0x94) out += '-';
                else if (t == 0xA6)              out += "...";
                else                             out += '?';
                i += 3;
            } else {
                const std::size_t len = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
                out += '?';
                i += len;
            }
        }
        return out;
    }

    bool parseInt(const std::string& s, int& out) {
        if (s.empty()) return false;
        char* end = nullptr;
        const long value = std::strtol(s.c_str(), &end, 10);
        if (end == s.c_str() || *end != '\0') return false;
        out = static_cast<int>(value);
        return true;
    }

    int clampEffect(int value) {
        if (value >  kMaxEffectAmount) return  kMaxEffectAmount;
        if (value < -kMaxEffectAmount) return -kMaxEffectAmount;
        return value;
    }
}

bool Trail::loadFromFile(const std::string& path) {
    m_events.clear();
    m_eventCount = 0;
    m_warnings.clear();
    reset();

    std::ifstream file(path);
    if (!file.is_open()) {
        m_warnings.push_back("Could not open event file: " + path);
        return false;
    }

    // This part of the code is for the state of the event / choice currently being read.
    bool inEvent = false;
    bool inChoice = false;
    bool eventOk = true;
    int eventLine = 0;
    int day = 0;
    bool partSet = false;
    TimePart part = TimePart::Dawn;
    std::optional<Condition> eventCondition;
    std::string text;
    std::vector<Choice> choices;
    int choiceLine = 0;
    std::string label;
    std::string result;
    Effects effects;
    SceneId eventScene = SceneId::Trail;
    ActionId choiceAction = ActionId::None;

    auto warn = [&](int line, const std::string& message) {
        m_warnings.push_back(path + " line " + std::to_string(line) + ": " + message);
    };

    auto finishChoice = [&]() {
        if (!inChoice) return;
        inChoice = false;
        if (label.empty() || result.empty()) {
            warn(choiceLine, "a choice needs both 'label' and 'result'; the whole event is skipped");
            eventOk = false;
        } else {
            if (label.size() > kMaxLabelChars)   warn(choiceLine, "label is long and may be cut off on its button");
            if (result.size() > kMaxResultChars) warn(choiceLine, "result text is long and may be cut off");
            choices.emplace_back(label, result, effects, choiceAction);
        }
        label.clear();
        result.clear();
        effects = Effects();
        choiceAction = ActionId::None;
    };

    auto finishEvent = [&]() {
        finishChoice();
        if (!inEvent) return;
        inEvent = false;

        if (day < 1)     { warn(eventLine, "event needs a 'day' of 1 or more; skipped"); eventOk = false; }
        if (!partSet)    { warn(eventLine, "event needs a valid 'part'; skipped");        eventOk = false; }
        if (text.empty()){ warn(eventLine, "event needs 'text'; skipped");                eventOk = false; }
        if (choices.empty() || choices.size() > kMaxChoices) {
            warn(eventLine, "event needs between 1 and 4 choices; skipped");
            eventOk = false;
        }
        if (eventOk && text.size() > kMaxTextChars) {
            warn(eventLine, "event text is long and may be cut off");
        }

        if (eventOk) {
            const std::pair<int, int> key(day, static_cast<int>(part));
            std::vector<Event>& slot = m_events[key];

            bool duplicate = false;
            for (const Event& existing : slot) {
                const std::optional<Condition>& other = existing.getCondition();
                if (other.has_value() == eventCondition.has_value() &&
                    (!other.has_value() || *other == *eventCondition)) {
                    duplicate = true;
                }
            }
            if (duplicate) {
                warn(eventLine, "duplicate event for that day, part and condition; keeping the first one");
                if (slot.empty()) m_events.erase(key);
            } else {
                slot.emplace_back(day, part, text, choices, eventCondition, eventScene);
                ++m_eventCount;
            }
        }
    };

    auto startEvent = [&](int line) {
        inEvent = true;
        eventOk = true;
        eventLine = line;
        day = 0;
        partSet = false;
        eventCondition.reset();
        eventScene = SceneId::Trail;
        text.clear();
        choices.clear();
    };

    std::string raw;
    int lineNumber = 0;
    while (std::getline(file, raw)) {
        ++lineNumber;
        if (lineNumber == 1 && raw.compare(0, 3, "\xEF\xBB\xBF") == 0) raw.erase(0, 3);  // This part of the code is for UTF-8 BOM

        const std::string line = trim(toAscii(raw));  // This part of the code is so trim also removes a Windows '\r'
        if (line.empty() || line[0] == '#') continue;

        const std::string lower = toLower(line);
        if (lower == "[event]") {
            finishEvent();
            startEvent(lineNumber);
            continue;
        }
        if (lower == "[choice]") {
            if (!inEvent) {
                warn(lineNumber, "[CHOICE] found before any [EVENT]; ignored");
                continue;
            }
            finishChoice();
            inChoice = true;
            choiceLine = lineNumber;
            continue;
        }

        const std::size_t equals = line.find('=');
        if (equals == std::string::npos) {
            warn(lineNumber, "expected 'key = value'; line ignored");
            continue;
        }
        const std::string key = toLower(trim(line.substr(0, equals)));
        const std::string value = trim(line.substr(equals + 1));

        if (!inEvent) {
            warn(lineNumber, "'" + key + "' found outside an [EVENT]; ignored");
            continue;
        }

        if (inChoice) {
            int* target = nullptr;
            if (key == "food")        target = &effects.food;
            else if (key == "health") target = &effects.health;
            else if (key == "sanity") target = &effects.sanity;
            else if (key == "wagon")  target = &effects.wagon;

            if (key == "label") {
                label = value;
            } else if (key == "result") {
                result = value;
            } else if (key == "action") {
                // This part of the code keeps the animations are decoration: a bad name never skips the event, it just gets no animation.
                if (!parseAction(value, choiceAction)) {
                    warn(lineNumber, "unknown action '" + value + "'; the choice gets no animation");
                    choiceAction = ActionId::None;
                }
            } else if (key == "member") {
                const int index = findMemberIndex(value);
                if (index >= 0) {
                    effects.member = memberTemplates()[index].name;
                } else {
                    warn(lineNumber, "unknown member '" + value + "' (use Ma, Pa, Junior or Granny); the whole event is skipped");
                    eventOk = false;
                }
            } else if (target) {
                int number = 0;
                if (parseInt(value, number)) {
                    *target = clampEffect(number);
                } else {
                    warn(lineNumber, "'" + key + "' must be a whole number; the whole event is skipped");
                    eventOk = false;
                }
            } else {
                warn(lineNumber, "unknown choice key '" + key + "'; ignored");
            }
        } else {
            if (key == "day") {
                int number = 0;
                if (parseInt(value, number) && number >= 1) {
                    day = number;
                } else {
                    warn(lineNumber, "'day' must be a whole number of 1 or more; the whole event is skipped");
                    eventOk = false;
                }
            } else if (key == "part") {
                if (parsePart(value, part)) {
                    partSet = true;
                } else {
                    warn(lineNumber, "unknown part '" + value + "' (use Dawn, Mid-Morning, Noon, Evening or Night); the whole event is skipped");
                    eventOk = false;
                }
            } else if (key == "scene") {
                if (!parseScene(value, eventScene)) {
                    warn(lineNumber, "unknown scene '" + value + "'; the event uses the default scene");
                    eventScene = SceneId::Trail;
                }
            } else if (key == "text") {
                text = value;
            } else if (key == "requires") {
                Condition condition;
                if (parseCondition(value, condition)) {
                    eventCondition = condition;
                } else {
                    warn(lineNumber, "unknown condition '" + value + "' (use LowFood, LowWagon or LowHealth); the whole event is skipped");
                    eventOk = false;
                }
            } else {
                warn(lineNumber, "unknown event key '" + key + "'; ignored");
            }
        }
    }
    finishEvent();

    // This part of the code is for a slot that only has desperate variants would be empty most of the time.
    for (const auto& entry : m_events) {
        bool hasDefault = false;
        for (const Event& event : entry.second) {
            if (!event.getCondition().has_value()) hasDefault = true;
        }
        if (!hasDefault) {
            m_warnings.push_back(path + ": Day " + std::to_string(entry.first.first) + ", " +
                                 partName(static_cast<TimePart>(entry.first.second)) +
                                 " has only conditional variants and no normal event");
        }
    }

    if (m_eventCount == 0) {
        m_warnings.push_back(path + ": no valid events were found");
        return false;
    }
    return true;
}

const Event* Trail::getEvent(int day, TimePart part, ConditionSet active) const {
    const auto it = m_events.find(std::make_pair(day, static_cast<int>(part)));
    if (it == m_events.end()) return nullptr;

    const Event* normal = nullptr;
    for (const Event& event : it->second) {
        if (!event.getCondition().has_value()) {
            if (!normal) normal = &event;
        } else if (active.has(*event.getCondition())) {
            return &event;  // This part of the code is for an active desperate variant replaces the normal event
        }
    }
    return normal;
}

bool Trail::hasDay(int day) const {
    const auto it = m_events.lower_bound(std::make_pair(day, 0));
    return it != m_events.end() && it->first.first == day;
}

int Trail::lastDay() const {
    return m_events.empty() ? 1 : m_events.rbegin()->first.first;
}

void Trail::advance() {
    if (m_part == TimePart::Night) {
        m_part = TimePart::Dawn;
        ++m_day;
    } else {
        m_part = static_cast<TimePart>(static_cast<int>(m_part) + 1);
    }
}

void Trail::reset() {
    m_day = 1;
    m_part = TimePart::Dawn;
}
