#pragma once
#include <string>
#include <vector>

// This part of the code keeps score of how often the narrator misled the player, so the game can reveal it.
//
// Playtesters did not notice the narrator lying, because the lies were subtle and never
// pointed out. This class compares what the game WOULD have said (the truth) with what it
// actually showed (the possibly lying version), counts the differences and keeps one
// example of each kind. The end-of-day "truth check" and the ending use it to show the
// player what they were told versus what was true.
class LieTracker {
public:
    // This part of the code is for one HUD bar: what it showed and what the real value was.
    struct Bar {
        std::string name;
        int shown = 0;
        int truth = 0;
    };

    // This part of the code is for the first difference found between true text and shown text.
    struct TextLie {
        std::string truth;     // This part of the code is for the word that should have been there (empty for an aside)
        std::string shown;     // This part of the code is for the word the player saw, or the aside the game added
        bool isAside = false;  // This part of the code is true if the game added a comment rather than swapping a word
    };

    // This part of the code compares the true text with the shown text. Returns true, and fills 'out', if the
    // shown text differs. A swapped word is preferred over an added aside as the example.
    static bool findTextLie(const std::string& truth, const std::string& shown, TextLie& out);

    void recordText(const std::string& truth, const std::string& shown);
    void recordBars(const std::vector<Bar>& bars);   // counts once per screen, if any bar was wrong

    void startDay();   // This part of the code sets a new day: the day's tally starts again (the journey total keeps counting)
    void reset();      // a new game

    int dayCount() const   { return m_day.count(); }
    int totalCount() const { return m_total.count(); }

    // This part of the code is for plain-language reports. Always true, always short enough for the bottom panel.
    std::string dayReport() const;
    std::string journeyReport() const;   // This part of the code: remains empty if the narrator never misled the player

private:
    struct Tally {
        int textLies = 0;
        int barLies = 0;
        bool haveSwap = false;
        bool haveAside = false;
        bool haveBar = false;
        TextLie swap;
        TextLie aside;
        Bar bar;
        int count() const { return textLies + barLies; }
    };

    static void addText(Tally& tally, const TextLie& lie);
    static void addBar(Tally& tally, const Bar& bar);

    Tally m_day;
    Tally m_total;
};
