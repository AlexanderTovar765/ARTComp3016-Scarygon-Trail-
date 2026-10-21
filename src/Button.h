#pragma once
#include <SDL3/SDL.h>
#include <string>
#include "TextRenderer.h"

// This part of the code is for the clickable rectangle with a text label, its in to make numbered choices in the
// bottom panel. It only knows how to draw itself and answer "is this point inside
// me?". Once clicked, the game decides what it means.
class Button {
public:
    Button(std::string label, SDL_FRect rect);

    void setRect(const SDL_FRect& rect) { m_rect = rect; }
    void setHovered(bool hovered)       { m_hovered = hovered; }

    bool contains(float x, float y) const;
    const std::string& getLabel() const { return m_label; }

    // this part of the code is for the number and having it shown as a prefix (e.g. 1.ford the river) and makes a shortcut.
    void draw(SDL_Renderer* renderer, const TextRenderer& text, int number) const;

private:
    std::string m_label;
    SDL_FRect   m_rect;
    bool        m_hovered = false;
};
