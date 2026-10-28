#pragma once
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Animations.h"
#include "Conditions.h"
#include "Effects.h"
#include "TimeOfDay.h"

// One option the player can pick: the button label, what happens, and how it
// changes the party's stats.
class Choice {
public:
    Choice(std::string label, std::string resultText, Effects effects, ActionId action = ActionId::None)
        : m_label(std::move(label)), m_resultText(std::move(resultText)), m_effects(effects), m_action(action) {}

    const std::string& getLabel() const      { return m_label; }
    const std::string& getResultText() const { return m_resultText; }
    const Effects&     getEffects() const    { return m_effects; }
    ActionId           getAction() const     { return m_action; }   // the animation played with the result

private:
    std::string m_label;
    std::string m_resultText;
    Effects     m_effects;
    ActionId    m_action;
};

// A narrative moment: it belongs to one part of one day and offers up to 4 Choices.
// An event may be a "desperate variant": it only appears (in place of the normal event
// for that slot) while its Condition is active, e.g. LowFood.
class Event {
public:
    Event(int day, TimePart part, std::string text, std::vector<Choice> choices,
          std::optional<Condition> condition = std::nullopt, SceneId scene = SceneId::Trail)
        : m_day(day), m_part(part), m_text(std::move(text)), m_choices(std::move(choices)),
          m_condition(condition), m_scene(scene) {}

    int getDay() const                         { return m_day; }
    TimePart getPart() const                   { return m_part; }
    const std::string& getText() const         { return m_text; }
    const std::vector<Choice>& getChoices() const { return m_choices; }
    const std::optional<Condition>& getCondition() const { return m_condition; }
    SceneId getScene() const                   { return m_scene; }   // the animation shown while the event is on screen

private:
    int m_day;
    TimePart m_part;
    std::string m_text;
    std::vector<Choice> m_choices;
    std::optional<Condition> m_condition;
    SceneId m_scene;
};
