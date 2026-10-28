#include "LieTracker.h"

#include <cctype>
#include <sstream>

namespace {
    std::vector<std::string> splitWords(const std::string& text) {
        std::vector<std::string> words;
        std::istringstream in(text);
        std::string word;
        while (in >> word) words.push_back(word);
        return words;
    }

    // Just the letters, so "wagon," and "hearse," compare as words, not punctuation.
    std::string lettersOnly(const std::string& word) {
        std::string out;
        for (unsigned char c : word) {
            if (std::isalpha(c)) out += static_cast<char>(c);
        }
        return out;
    }

    std::string countText(int n) {
        return n == 1 ? std::string("once") : std::to_string(n) + " times";
    }
}

bool LieTracker::findTextLie(const std::string& truth, const std::string& shown, TextLie& out) {
    const std::vector<std::string> t = splitWords(truth);
    const std::vector<std::string> s = splitWords(shown);

    bool haveSwap = false;
    bool haveAside = false;
    TextLie swap;
    TextLie aside;

    std::size_t i = 0;  // position in the true text
    std::size_t j = 0;  // position in the shown text
    while (j < s.size()) {
        if (i < t.size() && t[i] == s[j]) {          // same word: nothing to see
            ++i;
            ++j;
        } else if (s[j][0] == '(') {                 // a comment the game added, in brackets
            std::string text;
            while (j < s.size()) {
                if (!text.empty()) text += ' ';
                text += s[j];
                const bool closes = s[j].back() == ')';
                ++j;
                if (closes) break;
            }
            if (!haveAside) {
                haveAside = true;
                aside.shown = text;
                aside.isAside = true;
            }
        } else if (i < t.size()) {                   // a word that was swapped
            const std::string a = lettersOnly(t[i]);
            const std::string b = lettersOnly(s[j]);
            if (!haveSwap && !a.empty() && !b.empty() && a != b) {
                haveSwap = true;
                swap.truth = a;
                swap.shown = b;
            }
            ++i;
            ++j;
        } else {                                     // extra words with nothing to compare to
            ++j;
        }
    }

    if (haveSwap)  { out = swap;  return true; }
    if (haveAside) { out = aside; return true; }
    return false;
}

void LieTracker::addText(Tally& tally, const TextLie& lie) {
    ++tally.textLies;
    if (lie.isAside) {
        if (!tally.haveAside) { tally.haveAside = true; tally.aside = lie; }
    } else if (!tally.haveSwap) {
        tally.haveSwap = true;
        tally.swap = lie;
    }
}

void LieTracker::addBar(Tally& tally, const Bar& bar) {
    ++tally.barLies;
    if (!tally.haveBar) { tally.haveBar = true; tally.bar = bar; }
}

void LieTracker::recordText(const std::string& truth, const std::string& shown) {
    TextLie lie;
    if (!findTextLie(truth, shown, lie)) return;
    addText(m_day, lie);
    addText(m_total, lie);
}

void LieTracker::recordBars(const std::vector<Bar>& bars) {
    for (const Bar& bar : bars) {
        if (bar.shown != bar.truth) {   // one wrong bar is enough to count this screen
            addBar(m_day, bar);
            addBar(m_total, bar);
            return;
        }
    }
}

void LieTracker::startDay() { m_day = Tally(); }

void LieTracker::reset() {
    m_day = Tally();
    m_total = Tally();
}

std::string LieTracker::dayReport() const {
    if (m_day.count() == 0) return "Truth check: the narrator told the truth today.";

    std::string report = "Truth check: the narrator misled you " + countText(m_day.count()) + " today.";
    if (m_day.haveSwap) {
        report += " It said \"" + m_day.swap.shown + "\" where the truth was \"" + m_day.swap.truth + "\".";
    } else if (m_day.haveAside) {
        report += " It also added comments you never asked for.";
    }
    if (m_day.haveBar) {
        report += " The " + m_day.bar.name + " bar showed " + std::to_string(m_day.bar.shown) +
                  " when the truth was " + std::to_string(m_day.bar.truth) + ".";
    }
    return report;
}

std::string LieTracker::journeyReport() const {
    if (m_total.count() == 0) return "";
    return "The narrator misled you " + countText(m_total.count()) + ".";
}
