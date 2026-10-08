#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>

// Thin wrapper around SDL3's built-in 8x8 bitmap font (SDL_RenderDebugText).
// The rest of the game never calls SDL text functions directly, so if SDL3_ttf
// is approved later only this class needs to change.
//
// Notes:
//  - Only plain ASCII is supported (use straight quotes ' and ", not curly ones).
//  - Text colour is the renderer's current draw colour (SDL_SetRenderDrawColor).
//  - The font is monospaced, so wrapping is simple character counting.
class TextRenderer {
public:
    // Draw a single line of text with its top-left corner at (x, y).
    void drawLine(SDL_Renderer* renderer, const std::string& text,
                  float x, float y, float scale = 1.0f) const;

    // Draw word-wrapped text inside maxWidth pixels. Returns the number of lines drawn.
    // maxLines <= 0 means no limit; if text is cut off the last line ends in "...".
    int drawWrapped(SDL_Renderer* renderer, const std::string& text,
                    float x, float y, float maxWidth,
                    float scale = 1.0f, int maxLines = 0) const;

    // Split text into lines that fit within maxWidth pixels at the given scale.
    std::vector<std::string> wrap(const std::string& text, float maxWidth, float scale) const;

    float lineHeight(float scale) const;                 // one line plus spacing
    float blockHeight(int lines, float scale) const;     // height of 'lines' lines, no trailing gap
    float textWidth(const std::string& text, float scale) const;

private:
    int maxCharsPerLine(float maxWidth, float scale) const;

    static constexpr float kGlyphSize   = static_cast<float>(SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE);
    static constexpr float kLineSpacing = 4.0f;          // extra pixels between lines
};
