#include "TextRenderer.h"
#include <algorithm>
#include <sstream>

int TextRenderer::maxCharsPerLine(float maxWidth, float scale) const {
    return std::max(1, static_cast<int>(maxWidth / (kGlyphSize * scale)));
}

float TextRenderer::lineHeight(float scale) const {
    return kGlyphSize * scale + kLineSpacing;
}

float TextRenderer::blockHeight(int lines, float scale) const {
    if (lines <= 0) return 0.0f;
    return lines * kGlyphSize * scale + (lines - 1) * kLineSpacing;
}

float TextRenderer::textWidth(const std::string& text, float scale) const {
    return static_cast<float>(text.size()) * kGlyphSize * scale;
}

void TextRenderer::drawLine(SDL_Renderer* renderer, const std::string& text,
                            float x, float y, float scale) const {
    if (!renderer || text.empty()) return;

    // The debug font is always 8x8, so enlarge it by scaling the renderer, then
    // divide the position by the scale so it still lands at (x, y) on screen.
    SDL_SetRenderScale(renderer, scale, scale);
    SDL_RenderDebugText(renderer, x / scale, y / scale, text.c_str());
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

std::vector<std::string> TextRenderer::wrap(const std::string& text, float maxWidth, float scale) const {
    std::vector<std::string> lines;
    const int maxChars = maxCharsPerLine(maxWidth, scale);

    std::istringstream words(text);
    std::string word;
    std::string current;

    while (words >> word) {
        // A single word longer than a whole line can never fit, so hard-break it.
        while (static_cast<int>(word.size()) > maxChars) {
            if (!current.empty()) {
                lines.push_back(current);
                current.clear();
            }
            lines.push_back(word.substr(0, maxChars));
            word.erase(0, maxChars);
        }

        if (current.empty()) {
            current = word;
        } else if (static_cast<int>(current.size() + 1 + word.size()) <= maxChars) {
            current += " " + word;
        } else {
            lines.push_back(current);
            current = word;
        }
    }

    if (!current.empty()) lines.push_back(current);
    return lines;
}

int TextRenderer::drawWrapped(SDL_Renderer* renderer, const std::string& text,
                              float x, float y, float maxWidth,
                              float scale, int maxLines) const {
    std::vector<std::string> lines = wrap(text, maxWidth, scale);

    if (maxLines > 0 && static_cast<int>(lines.size()) > maxLines) {
        lines.resize(maxLines);

        // Mark the cut-off so truncated text is obvious rather than silently clipped.
        const size_t maxChars = static_cast<size_t>(maxCharsPerLine(maxWidth, scale));
        std::string& last = lines.back();
        if (maxChars > 3 && last.size() + 3 > maxChars) last.resize(maxChars - 3);
        last += "...";
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        drawLine(renderer, lines[i], x, y + static_cast<float>(i) * lineHeight(scale), scale);
    }
    return static_cast<int>(lines.size());
}
