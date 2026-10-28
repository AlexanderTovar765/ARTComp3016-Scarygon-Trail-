#pragma once
#include <cctype>
#include <cstdint>
#include <string>

// Situations that unlock a "desperate" version of an event (see ThresholdWatcher).
enum class Condition { LowFood = 0, LowWagon, LowHealth };

constexpr int kConditionCount = 3;

// A small set of conditions that are currently true.
class ConditionSet {
public:
    void set(Condition c, bool on = true) {
        if (on) m_bits = static_cast<std::uint8_t>(m_bits | bit(c));
        else    m_bits = static_cast<std::uint8_t>(m_bits & ~bit(c));
    }
    bool has(Condition c) const { return (m_bits & bit(c)) != 0; }
    bool any() const { return m_bits != 0; }
    void clear() { m_bits = 0; }

private:
    static std::uint8_t bit(Condition c) { return static_cast<std::uint8_t>(1U << static_cast<int>(c)); }
    std::uint8_t m_bits = 0;
};

inline const char* conditionName(Condition c) {
    static const char* const names[kConditionCount] = { "LowFood", "LowWagon", "LowHealth" };
    return names[static_cast<int>(c)];
}

namespace detail {
    inline std::string lettersOnly(const std::string& text) {
        std::string out;
        for (unsigned char ch : text) {
            if (std::isalpha(ch)) out += static_cast<char>(std::tolower(ch));
        }
        return out;
    }
}

// Reads a condition name from the data file ("LowFood", "low food" and "LOWFOOD" all work).
inline bool parseCondition(const std::string& text, Condition& out) {
    const std::string wanted = detail::lettersOnly(text);
    for (int i = 0; i < kConditionCount; ++i) {
        const Condition candidate = static_cast<Condition>(i);
        if (wanted == detail::lettersOnly(conditionName(candidate))) {
            out = candidate;
            return true;
        }
    }
    return false;
}
