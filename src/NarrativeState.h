#pragma once
#include <memory>
#include <string>

// The four party stats shown on the HUD.
enum class Stat { Food = 0, Health, Sanity, Wagon };

enum class NarrativeKind { Stable, Unreliable, Breakdown };

// STATE PATTERN: how much the game can be trusted, driven by the party's sanity.
// Game holds one of these through a base-class pointer and swaps it when sanity
// crosses a threshold. Each state decides how text and HUD numbers are presented:
//   Stable      - tells the truth
//   Unreliable  - swaps some words for wrong ones and nudges HUD numbers off
//   Breakdown   - lies freely, hides the numbers, and adds uncomfortable asides
// What the player's choices actually do is never changed. Only the presentation lies,
// and the "what happened" summary after each choice always stays truthful.
class NarrativeState {
public:
    // Sanity at or above this is Stable; at or above kUnreliableMin is Unreliable;
    // anything below is Breakdown. (0 sanity ends the run.)
    static constexpr int kStableMin     = 60;
    static constexpr int kUnreliableMin = 30;

    virtual ~NarrativeState() = default;

    virtual NarrativeKind kind() const = 0;
    virtual const char* name() const = 0;

    // Rewrites narrative text. 'seed' changes with every new screen but stays fixed
    // while one screen is shown, so the text never flickers or changes between frames.
    virtual std::string filterText(const std::string& text, unsigned seed) const = 0;

    // The value the HUD bar shows for a stat.
    virtual int shownValue(int actual, Stat stat, unsigned seed) const = 0;

    // The text printed next to a bar for a shown value.
    virtual std::string valueLabel(int shown) const { return std::to_string(shown); }

    // 0..1: how strongly the top-panel scene is tinted.
    virtual float dread() const = 0;

    // A one-off line shown when the game enters this state.
    virtual const char* enterMessage() const = 0;

    static NarrativeKind kindForSanity(int sanity);
    static std::unique_ptr<NarrativeState> create(NarrativeKind kind);
};
