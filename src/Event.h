#pragma once
#include <string>
#include <utility>
#include <vector>

// One option the player can pick for an Event.
// Week 3: add resource/sanity effects and apply(Party&).
class Choice {
public:
    explicit Choice(std::string label) : m_label(std::move(label)) {}

    const std::string& getLabel() const { return m_label; }

private:
    std::string m_label;
};

// A narrative moment on the trail: some text plus the choices offered.
// Week 3: load these from files in data/ instead of building them in code.
class Event {
public:
    Event(std::string text, std::vector<Choice> choices)
        : m_text(std::move(text)), m_choices(std::move(choices)) {}

    const std::string& getText() const { return m_text; }
    const std::vector<Choice>& getChoices() const { return m_choices; }

private:
    std::string m_text;
    std::vector<Choice> m_choices;
};
