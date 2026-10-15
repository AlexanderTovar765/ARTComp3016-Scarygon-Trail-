#pragma once
#include <SDL3/SDL.h>
#include "TextRenderer.h"
#include "TimeOfDay.h"

// Draws the top-panel artwork entirely from rectangles, circles and lines, so there
// are no image files to load (and nothing extra to approve or cite).
class SceneRenderer {
public:
    // The wagon on the road, with a sky that changes with the part of the day.
    // 'timeMs' drives the small animation (wheels, bobbing); 'dread' (0..1) tints
    // the whole scene red as the party loses its grip.
    void drawScene(SDL_Renderer* renderer, const SDL_FRect& area, TimePart part,
                   Uint64 timeMs, float dread) const;

    // One dot per part of every day along a strip: finished parts filled in,
    // the current part highlighted, upcoming parts hollow.
    void drawProgress(SDL_Renderer* renderer, const TextRenderer& text, const SDL_FRect& area,
                      int totalDays, int currentDay, TimePart currentPart) const;
};
