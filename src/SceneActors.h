#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL.h>

#include "Animations.h"
#include "TextRenderer.h"
#include "TimeOfDay.h"

// What the scene artwork needs to know to draw its actors and props.
// Text in the picture is drawn after the pixelated art is scaled up, so it stays readable.
struct DeferredText {
    std::string text;
    float x, y, scale;
    SDL_Color color;
};

struct StageContext {
    SDL_Renderer* r = nullptr;
    const TextRenderer* text = nullptr;
    SDL_FRect area{};
    float t = 0.0f;          // seconds, drives idle animation
    float horizon = 0.0f;
    float roadY = 0.0f;
    float groundY = 0.0f;    // where feet touch the ground
    float wagonX = 0.0f;
    TimePart part = TimePart::Dawn;

    std::vector<DeferredText>* deferred = nullptr;   // if set, label() queues text instead of drawing it

    float zoneX(float fraction) const { return area.x + fraction * area.w; }

    void label(const std::string& s, float x, float y, float scale) const {
        if (deferred) {
            SDL_Color col{ 0, 0, 0, 255 };
            SDL_GetRenderDrawColor(r, &col.r, &col.g, &col.b, &col.a);
            deferred->push_back({ s, x, y, scale, col });
        } else {
            text->drawLine(r, s, x, y, scale);
        }
    }
};

namespace stage {
    // An ox seen from the side. It is looking straight at you. It always is. 'blink' (0..1) closes its eyes.
    void drawOx(const StageContext& c, float x, float groundY, float legPhase, float blink = 0.0f);

    // The scene's idle "actor": a map seller, a deer, a ritual circle... drawn on the right of the picture.
    void drawSceneActor(const StageContext& c, SceneId scene);

    // The family's reaction to a choice: Pa walks out from the wagon and does the thing.
    // 'seconds' is how long the result has been on screen.
    void drawAction(const StageContext& c, ActionId action, float seconds);

    // A small mood badge in the corner: sparkles for a good result, a rain cloud for a bad one.
    void drawOutcomeBadge(const StageContext& c, int outcome, float seconds);
}
