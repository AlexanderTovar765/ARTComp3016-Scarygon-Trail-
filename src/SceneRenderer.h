#pragma once
#include <SDL3/SDL.h>
#include "Animations.h"
#include "PartyMember.h"
#include "TextRenderer.h"
#include "TimeOfDay.h"

// This part of the code draws the top-panel artwork entirely from rectangles, circles and lines, so there
// are no image files to load (and nothing extra to approve or cite).
class SceneRenderer {
public:
    // This part of the code is for the artwork is drawn small and scaled up with no smoothing, so every shape is made of chunky
    // square pixels (circles become stair-steps). Text is drawn afterwards at full sharpness.
    static constexpr int kPixelSize = 3;

    SceneRenderer() = default;
    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;
    ~SceneRenderer() { releaseResources(); }
    void releaseResources();   // This part of the code calls before the SDL_Renderer is destroyed

    // This part of the code is for the The wagon on the road, with a sky that changes with the part of the day.
    // 'timeMs' drives the small animation (wheels, bobbing); 'dread' (0..1) tints
    // the whole scene red as the party loses its grip. 'travel' (0..1) drives the wagon
    // across the scene (used on the title screen); leave it negative to park it at the left.
    void drawScene(SDL_Renderer* renderer, const SDL_FRect& area, TimePart part,
                   Uint64 timeMs, float dread, float travel = -1.0f) const;

    // This part of the code is for what is happening on the road right now: an event's scene (a map seller, a deer, a river...),
    // and, once a choice has been made, the family's reaction to it. 'actionSeconds' is how long the
    // result has been on screen; 'outcome' (-1 bad, 0 neutral, 1 good) picks the mood badge.
    struct Stage {
        SceneId scene = SceneId::Trail;
        ActionId action = ActionId::None;
        float actionSeconds = 0.0f;
        int outcome = 0;
    };
    void drawStage(SDL_Renderer* renderer, const TextRenderer& text, const SDL_FRect& area, TimePart part,
                   Uint64 timeMs, float dread, const Stage& stage) const;

    // This part of the code is for a small face for one party member (0 = Ma, 1 = Pa, 2 = Junior, 3 = Granny).
    // Ill members look green and sweaty; the dead are grey, with crossed-out eyes and a halo.
    void drawPortrait(SDL_Renderer* renderer, const SDL_FRect& area, int memberIndex,
                      PartyMember::Status status) const;

    // This part of the code is for one dot per part of every day along a strip: finished parts filled in,
    // the current part highlighted, upcoming parts hollow.
    void drawProgress(SDL_Renderer* renderer, const TextRenderer& text, const SDL_FRect& area,
                      int totalDays, int currentDay, TimePart currentPart) const;

private:
    // This part of the code switches drawing to a small off-screen picture; endPixelPass() scales it up into 'area'.
    bool beginPixelPass(SDL_Renderer* renderer) const;
    void endPixelPass(SDL_Renderer* renderer, const SDL_FRect& area) const;

    mutable SDL_Texture* m_pixelTexture = nullptr;
    mutable int m_pixelW = 0, m_pixelH = 0;
    mutable float m_savedScaleX = 1.0f, m_savedScaleY = 1.0f;
    mutable bool m_inPass = false;
};
