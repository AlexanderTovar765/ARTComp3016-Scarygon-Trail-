#pragma once
#include <cctype>
#include <string>

// This part of the code sets the five parts of every day, in order. One Event happens in each part.
// To rename a part, change its entry in partName() below AND the 'part =' lines
// in data/events.txt (parsePart ignores case, spaces and dashes).
enum class TimePart { Dawn = 0, MidMorning, Noon, Evening, Night };

constexpr int kPartsPerDay = 5;

inline const char* partName(TimePart part) {
    static const char* const names[kPartsPerDay] = { "Dawn", "Mid-Morning", "Noon", "Evening", "Night" };
    return names[static_cast<int>(part)];
}

namespace detail {
    // This part of the code is for lower-case letters only, so "Mid-Morning", "mid morning" and "MIDMORNING" all match.
    inline std::string letters(const std::string& text) {
        std::string out;
        for (unsigned char c : text) {
            if (std::isalpha(c)) out += static_cast<char>(std::tolower(c));
        }
        return out;
    }
}

// This part of the code converts text from the data file into a TimePart. Returns false if it isn't one.
inline bool parsePart(const std::string& text, TimePart& out) {
    const std::string wanted = detail::letters(text);
    for (int i = 0; i < kPartsPerDay; ++i) {
        const TimePart candidate = static_cast<TimePart>(i);
        if (wanted == detail::letters(partName(candidate))) {
            out = candidate;
            return true;
        }
    }
    return false;
}
