#pragma once
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Conditions.h"
#include "Effects.h"
#include "TimeOfDay.h"

// This part of the code makes one option the player can pick: the button label, what happens, and how it will
// change the party's stats.
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

// This part of the code is for the narrative moments: it belongs to one portion of one day and gives up to 4 Choices.
// An event may be a "desperate variant": it will only appear (in place of the normal event
// for that slot) while its condition is active, like if LowFood happens too early.
class Event {
public:
    Event(int day, TimePart part, std::string text, std::vector<Choice> choices,
          std::optional<Condition> condition = std::nullopt)
        : m_day(day), m_part(part), m_text(std::move(text)), m_choices(std::move(choices)),
          m_condition(condition) {}

    int getDay() const                         { return m_day; }
    TimePart getPart() const                   { return m_part; }
    const std::string& getText() const         { return m_text; }
    const std::vector<Choice>& getChoices() const { return m_choices; }
    const std::optional<Condition>& getCondition() const { return m_condition; }

private:
    int m_day;
    TimePart m_part;
    std::string m_text;
    std::vector<Choice> m_choices;
    std::optional<Condition> m_condition;
};
