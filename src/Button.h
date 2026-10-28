#pragma once
#include <SDL3/SDL.h>
#include <string>
#include "TextRenderer.h"

// A clickable rectangle with a text label, used for the numbered choices in the
// bottom panel. It only knows how to draw itself and answer "is this point inside
// me?". Game decides what clicking it means.
class Button {
public:
    Button(std::string label, SDL_FRect rect);

    void setRect(const SDL_FRect& rect) { m_rect = rect; }
    void setHovered(bool hovered)       { m_hovered = hovered; }

    bool contains(float x, float y) const;
    const std::string& getLabel() const { return m_label; }

    // 'number' is shown as a prefix ("1. Ford the river") and matches the keyboard shortcut.
    void draw(SDL_Renderer* renderer, const TextRenderer& text, int number) const;

private:
    std::string m_label;
    SDL_FRect   m_rect;
    bool        m_hovered = false;
};
