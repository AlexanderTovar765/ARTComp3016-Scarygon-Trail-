#pragma once
#include <SDL3/SDL.h>
#include "PartyMember.h"
#include "TextRenderer.h"
#include "TimeOfDay.h"

// Draws the top-panel artwork entirely from rectangles, circles and lines, so there
// are no image files to load (and nothing extra to approve or cite).
class SceneRenderer {
public:
    // The wagon on the road, with a sky that changes with the part of the day.
    // 'timeMs' drives the small animation (wheels, bobbing); 'dread' (0..1) tints
    // the whole scene red as the party loses its grip. 'travel' (0..1) drives the wagon
    // across the scene (used on the title screen); leave it negative to park it at the left.
    void drawScene(SDL_Renderer* renderer, const SDL_FRect& area, TimePart part,
                   Uint64 timeMs, float dread, float travel = -1.0f) const;

    // A small face for one party member (0 = Ma, 1 = Pa, 2 = Junior, 3 = Granny).
    // Ill members look green and sweaty; the dead are grey, with crossed-out eyes and a halo.
    void drawPortrait(SDL_Renderer* renderer, const SDL_FRect& area, int memberIndex,
                      PartyMember::Status status) const;

    // One dot per part of every day along a strip: finished parts filled in,
    // the current part highlighted, upcoming parts hollow.
    void drawProgress(SDL_Renderer* renderer, const TextRenderer& text, const SDL_FRect& area,
                      int totalDays, int currentDay, TimePart currentPart) const;
};
