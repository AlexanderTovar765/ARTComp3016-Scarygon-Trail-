#pragma once
#include <string>
#include <utility>
#include <vector>

#include "Effects.h"
#include "TimeOfDay.h"

// One option the player can pick: the button label, what happens, and how it
// changes the party's stats.
class Choice {
public:
    Choice(std::string label, std::string resultText, Effects effects)
        : m_label(std::move(label)), m_resultText(std::move(resultText)), m_effects(effects) {}

    const std::string& getLabel() const      { return m_label; }
    const std::string& getResultText() const { return m_resultText; }
    const Effects&     getEffects() const    { return m_effects; }

private:
    std::string m_label;
    std::string m_resultText;
    Effects     m_effects;
};

// A narrative moment: it belongs to one part of one day and offers up to 4 Choices.
class Event {
public:
    Event(int day, TimePart part, std::string text, std::vector<Choice> choices)
        : m_day(day), m_part(part), m_text(std::move(text)), m_choices(std::move(choices)) {}

    int getDay() const                         { return m_day; }
    TimePart getPart() const                   { return m_part; }
    const std::string& getText() const         { return m_text; }
    const std::vector<Choice>& getChoices() const { return m_choices; }

private:
    int m_day;
    TimePart m_part;
    std::string m_text;
    std::vector<Choice> m_choices;
};
