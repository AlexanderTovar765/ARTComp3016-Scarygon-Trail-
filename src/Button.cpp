#include "Button.h"
#include <algorithm>

namespace {
    constexpr float kLabelScale   = 2.0f;  // 8px font drawn at 16px
    constexpr float kLabelPadding = 10.0f;
    constexpr int   kMaxLabelLines = 2;
}

Button::Button(std::string label, SDL_FRect rect)
    : m_label(std::move(label)), m_rect(rect) {}

bool Button::contains(float x, float y) const {
    const SDL_FPoint point{ x, y };
    return SDL_PointInRectFloat(&point, &m_rect);
}

void Button::draw(SDL_Renderer* renderer, const TextRenderer& text, int number) const {
    // Background: slightly brighter when the mouse is over it.
    if (m_hovered) {
        SDL_SetRenderDrawColor(renderer, 250, 235, 190, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 232, 220, 190, 255);
    }
    SDL_RenderFillRect(renderer, &m_rect);

    // Border: doubled up when hovered so it reads without relying on colour alone.
    SDL_SetRenderDrawColor(renderer, 90, 70, 50, 255);
    SDL_RenderRect(renderer, &m_rect);
    if (m_hovered) {
        const SDL_FRect inner{ m_rect.x + 1.0f, m_rect.y + 1.0f, m_rect.w - 2.0f, m_rect.h - 2.0f };
        SDL_RenderRect(renderer, &inner);
    }

    // Label, wrapped to the button width and centred vertically.
    const std::string label = std::to_string(number) + ". " + m_label;
    const float maxWidth = m_rect.w - 2.0f * kLabelPadding;
    const int lineCount = std::min(static_cast<int>(text.wrap(label, maxWidth, kLabelScale).size()),
                                   kMaxLabelLines);
    const float y = m_rect.y + (m_rect.h - text.blockHeight(lineCount, kLabelScale)) / 2.0f;

    SDL_SetRenderDrawColor(renderer, 50, 35, 25, 255);
    text.drawWrapped(renderer, label, m_rect.x + kLabelPadding, y, maxWidth, kLabelScale, kMaxLabelLines);
}
