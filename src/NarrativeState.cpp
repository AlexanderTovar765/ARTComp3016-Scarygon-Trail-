#include "NarrativeState.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <vector>

namespace {
    // ---- Tuning knobs --------------------------------------------------------
    constexpr unsigned kUnreliableSwapChance = 35;   // % of swappable words replaced
    constexpr unsigned kBreakdownSwapChance  = 85;
    constexpr int      kUnreliableJitter     = 10;   // HUD numbers drift by up to +/- this
    constexpr int      kBreakdownJitter      = 30;
    constexpr std::size_t kMaxTextChars      = 220;  // keeps lying text inside the panel

    // Small deterministic hash so the same (seed, position) always gives the same answer.
    std::uint32_t mix(std::uint32_t x) {
        x ^= x >> 16; x *= 0x7feb352dU;
        x ^= x >> 15; x *= 0x846ca68bU;
        x ^= x >> 16;
        return x;
    }

    unsigned roll(unsigned seed, unsigned salt) {  // 0..99
        return mix(seed * 2654435761U + salt * 40503U + 17U) % 100U;
    }

    struct Swap { const char* from; const char* to; };
    const Swap kSwaps[] = {
        { "oxen",     "audience"    },
        { "ox",       "critic"      },
        { "wagon",    "hearse"      },
        { "family",   "congregation"},
        { "son",      "intern"      },
        { "food",     "evidence"    },
        { "river",    "mouth"       },
        { "road",     "corridor"    },
        { "deer",     "lawyer"      },
        { "coat",     "alibi"       },
        { "map",      "confession"  },
        { "maps",     "confessions" },
        { "stranger", "landlord"    },
        { "stew",     "paperwork"   },
        { "fire",     "meeting"     },
    };

    const char* const kAsides[] = {
        "(It is fine.)",
        "(They know.)",
        "(You did this.)",
        "(Keep going.)",
        "(Nobody is coming.)",
    };

    std::string lower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    const char* findSwap(const std::string& lowerWord) {
        for (const Swap& swap : kSwaps) {
            if (lowerWord == swap.from) return swap.to;
        }
        return nullptr;
    }

    // Keep the original's capitalisation: "Wagon" -> "Hearse", "WAGON" -> "HEARSE".
    std::string matchCase(const std::string& original, std::string replacement) {
        const bool allCaps = original.size() > 1 &&
            std::all_of(original.begin(), original.end(),
                        [](unsigned char c) { return std::isupper(c) != 0; });
        if (allCaps) {
            for (char& c : replacement) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        } else if (std::isupper(static_cast<unsigned char>(original[0]))) {
            replacement[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(replacement[0])));
        }
        return replacement;
    }

    // Replaces some swappable words with wrong ones. Each word has 'chancePercent' of being
    // swapped, but if the dice pick none, one is swapped anyway: in a lying state the
    // text always lies at least once (when it has anything to lie about).
    std::string swapWords(const std::string& text, unsigned seed, unsigned chancePercent) {
        struct Token { std::size_t start; std::size_t length; const char* replacement; };
        std::vector<Token> eligible;

        std::size_t i = 0;
        while (i < text.size()) {
            if (std::isalpha(static_cast<unsigned char>(text[i]))) {
                std::size_t j = i;
                while (j < text.size() && std::isalpha(static_cast<unsigned char>(text[j]))) ++j;
                if (const char* replacement = findSwap(lower(text.substr(i, j - i)))) {
                    eligible.push_back({ i, j - i, replacement });
                }
                i = j;
            } else {
                ++i;
            }
        }
        if (eligible.empty()) return text;

        std::vector<bool> chosen(eligible.size(), false);
        bool any = false;
        for (std::size_t k = 0; k < eligible.size(); ++k) {
            chosen[k] = roll(seed, static_cast<unsigned>(k)) < chancePercent;
            any = any || chosen[k];
        }
        if (!any) chosen[mix(seed * 131U + 5U) % eligible.size()] = true;

        std::string out;
        out.reserve(text.size() + 16);
        std::size_t cursor = 0;
        for (std::size_t k = 0; k < eligible.size(); ++k) {
            if (!chosen[k]) continue;
            const Token& t = eligible[k];
            out += text.substr(cursor, t.start - cursor);
            out += matchCase(text.substr(t.start, t.length), t.replacement);
            cursor = t.start + t.length;
        }
        out += text.substr(cursor);
        return out;
    }

    // Moves a HUD value by a repeatable random amount, staying within 0..100.
    int jitter(int actual, int range, Stat stat, unsigned seed) {
        const std::uint32_t r = mix(seed * 40503U + static_cast<unsigned>(stat) * 7919U + 11U);
        const int offset = static_cast<int>(r % static_cast<unsigned>(2 * range + 1)) - range;
        return std::max(0, std::min(100, actual + offset));
    }

    // ---- The three states ----------------------------------------------------
    class StableNarration : public NarrativeState {
    public:
        NarrativeKind kind() const override { return NarrativeKind::Stable; }
        const char* name() const override { return "Stable"; }
        std::string filterText(const std::string& text, unsigned) const override { return text; }
        int shownValue(int actual, Stat, unsigned) const override { return actual; }
        float dread() const override { return 0.0f; }
        const char* enterMessage() const override { return "The world feels reassuringly accurate again."; }
    };

    class UnreliableNarration : public NarrativeState {
    public:
        NarrativeKind kind() const override { return NarrativeKind::Unreliable; }
        const char* name() const override { return "Unreliable"; }
        std::string filterText(const std::string& text, unsigned seed) const override {
            return swapWords(text, seed, kUnreliableSwapChance);
        }
        int shownValue(int actual, Stat stat, unsigned seed) const override {
            if (stat == Stat::Sanity) return actual;  // you can still tell how you are doing
            return jitter(actual, kUnreliableJitter, stat, seed);
        }
        float dread() const override { return 0.35f; }
        const char* enterMessage() const override {
            return "You have the distinct feeling that the world is being edited.";
        }
    };

    class BreakdownNarration : public NarrativeState {
    public:
        NarrativeKind kind() const override { return NarrativeKind::Breakdown; }
        const char* name() const override { return "Breakdown"; }
        std::string filterText(const std::string& text, unsigned seed) const override {
            std::string out = swapWords(text, seed, kBreakdownSwapChance);
            const char* aside = kAsides[mix(seed * 977U + 13U) % (sizeof(kAsides) / sizeof(kAsides[0]))];
            if (out.size() + 1 + std::char_traits<char>::length(aside) <= kMaxTextChars) {
                out += ' ';
                out += aside;
            }
            return out;
        }
        int shownValue(int actual, Stat stat, unsigned seed) const override {
            return jitter(actual, kBreakdownJitter, stat, seed);
        }
        std::string valueLabel(int) const override { return "??"; }
        float dread() const override { return 0.8f; }
        const char* enterMessage() const override {
            return "The trail has stopped pretending to be on your side.";
        }
    };
}

NarrativeKind NarrativeState::kindForSanity(int sanity) {
    if (sanity >= kStableMin)     return NarrativeKind::Stable;
    if (sanity >= kUnreliableMin) return NarrativeKind::Unreliable;
    return NarrativeKind::Breakdown;
}

std::unique_ptr<NarrativeState> NarrativeState::create(NarrativeKind kind) {
    switch (kind) {
    case NarrativeKind::Unreliable: return std::make_unique<UnreliableNarration>();
    case NarrativeKind::Breakdown:  return std::make_unique<BreakdownNarration>();
    case NarrativeKind::Stable:     break;
    }
    return std::make_unique<StableNarration>();
}
