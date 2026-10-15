#include "SceneRenderer.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace {
    constexpr float kPi = 3.14159265f;

    void setColor(SDL_Renderer* r, SDL_Color c, Uint8 alpha = 255) {
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, alpha);
    }

    void fillRect(SDL_Renderer* r, float x, float y, float w, float h) {
        const SDL_FRect rect{ x, y, w, h };
        SDL_RenderFillRect(r, &rect);
    }

    void fillCircle(SDL_Renderer* r, float cx, float cy, float radius) {
        for (float dy = -radius; dy <= radius; dy += 1.0f) {
            const float halfWidth = std::sqrt(std::max(0.0f, radius * radius - dy * dy));
            fillRect(r, cx - halfWidth, cy + dy, halfWidth * 2.0f, 1.0f);
        }
    }

    void strokeCircle(SDL_Renderer* r, float cx, float cy, float radius) {
        const int segments = 28;
        for (int i = 0; i < segments; ++i) {
            const float a0 = 2.0f * kPi * static_cast<float>(i) / segments;
            const float a1 = 2.0f * kPi * static_cast<float>(i + 1) / segments;
            SDL_RenderLine(r, cx + std::cos(a0) * radius, cy + std::sin(a0) * radius,
                              cx + std::cos(a1) * radius, cy + std::sin(a1) * radius);
        }
    }

    // A filled half-ellipse sitting on baseY (used for hills and the wagon's canvas cover).
    void fillDome(SDL_Renderer* r, float cx, float baseY, float halfWidth, float height) {
        for (float row = 0.0f; row < height; row += 1.0f) {
            const float t = row / height;
            const float w = halfWidth * std::sqrt(std::max(0.0f, 1.0f - t * t));
            fillRect(r, cx - w, baseY - row, w * 2.0f, 1.0f);
        }
    }

    struct Palette {
        SDL_Color sky;
        SDL_Color ground;
        SDL_Color hills;
        SDL_Color road;
    };

    // Indexed by TimePart: Dawn, Mid-Morning, Noon, Evening, Night.
    const Palette kPalettes[kPartsPerDay] = {
        { { 238, 178, 150, 255 }, { 120, 140,  80, 255 }, { 150, 125, 120, 255 }, { 170, 140,  95, 255 } },
        { { 170, 210, 235, 255 }, { 105, 150,  75, 255 }, { 130, 150, 150, 255 }, { 175, 145,  95, 255 } },
        { { 135, 196, 238, 255 }, {  95, 155,  70, 255 }, { 120, 150, 140, 255 }, { 185, 155, 100, 255 } },
        { { 226, 142,  92, 255 }, { 105, 100,  60, 255 }, { 120,  85,  80, 255 }, { 150, 115,  80, 255 } },
        { {  26,  32,  64, 255 }, {  35,  50,  50, 255 }, {  40,  42,  70, 255 }, {  70,  60,  55, 255 } },
    };

    // Positions of the sun (or moon) across the sky for each part, as fractions of the area.
    struct SkyBody { float x; float y; };
    const SkyBody kSkyBodies[kPartsPerDay] = {
        { 0.12f, 0.45f }, { 0.30f, 0.34f }, { 0.52f, 0.14f }, { 0.88f, 0.43f }, { 0.74f, 0.20f },
    };

    const SDL_Color kWagonWood   { 110,  72,  45, 255 };
    const SDL_Color kCanvas      { 236, 226, 200, 255 };
    const SDL_Color kOutline     {  50,  35,  25, 255 };
    const SDL_Color kOxHide      {  92,  70,  58, 255 };

    void drawWheel(SDL_Renderer* r, float cx, float cy, float radius, float angle) {
        setColor(r, kOutline);
        strokeCircle(r, cx, cy, radius);
        strokeCircle(r, cx, cy, radius - 1.0f);
        for (int i = 0; i < 4; ++i) {  // spokes
            const float a = angle + kPi * static_cast<float>(i) / 4.0f;
            SDL_RenderLine(r, cx - std::cos(a) * radius, cy - std::sin(a) * radius,
                              cx + std::cos(a) * radius, cy + std::sin(a) * radius);
        }
        fillCircle(r, cx, cy, 2.5f);
    }

    // An ox seen from the side. It is looking straight at you. It always is.
    void drawOx(SDL_Renderer* r, float x, float groundY, float legPhase) {
        setColor(r, kOxHide);
        fillRect(r, x, groundY - 44.0f, 58.0f, 26.0f);          // body
        fillRect(r, x + 54.0f, groundY - 52.0f, 20.0f, 18.0f);  // head
        const float stepA = std::sin(legPhase) * 2.0f;
        const float stepB = -stepA;
        fillRect(r, x + 4.0f,  groundY - 20.0f + stepA, 6.0f, 20.0f - stepA);   // legs
        fillRect(r, x + 18.0f, groundY - 20.0f + stepB, 6.0f, 20.0f - stepB);
        fillRect(r, x + 36.0f, groundY - 20.0f + stepA, 6.0f, 20.0f - stepA);
        fillRect(r, x + 48.0f, groundY - 20.0f + stepB, 6.0f, 20.0f - stepB);
        setColor(r, kOutline);
        SDL_RenderLine(r, x + 58.0f, groundY - 52.0f, x + 54.0f, groundY - 62.0f);  // horns
        SDL_RenderLine(r, x + 72.0f, groundY - 52.0f, x + 76.0f, groundY - 62.0f);
        SDL_SetRenderDrawColor(r, 245, 245, 245, 255);
        fillRect(r, x + 58.0f, groundY - 48.0f, 5.0f, 5.0f);   // eyes
        fillRect(r, x + 66.0f, groundY - 48.0f, 5.0f, 5.0f);
        setColor(r, kOutline);
        fillRect(r, x + 60.0f, groundY - 46.0f, 2.0f, 2.0f);   // pupils, fixed on the viewer
        fillRect(r, x + 66.0f, groundY - 46.0f, 2.0f, 2.0f);
    }
}

void SceneRenderer::drawScene(SDL_Renderer* r, const SDL_FRect& area, TimePart part,
                              Uint64 timeMs, float dread) const {
    const int index = static_cast<int>(part);
    const Palette& palette = kPalettes[index];
    const float horizon = area.y + area.h * 0.62f;
    const float seconds = static_cast<float>(timeMs) / 1000.0f;

    // Keep everything (the hills especially) inside the frame.
    const SDL_Rect clip{ static_cast<int>(area.x), static_cast<int>(area.y),
                         static_cast<int>(area.w), static_cast<int>(area.h) };
    SDL_SetRenderClipRect(r, &clip);

    // Sky
    setColor(r, palette.sky);
    fillRect(r, area.x, area.y, area.w, area.h);

    // Stars at night
    if (part == TimePart::Night) {
        SDL_SetRenderDrawColor(r, 235, 235, 255, 255);
        static const float stars[][2] = {
            {0.06f,0.10f},{0.15f,0.30f},{0.24f,0.08f},{0.33f,0.22f},{0.41f,0.12f},{0.50f,0.30f},
            {0.58f,0.07f},{0.66f,0.24f},{0.83f,0.09f},{0.91f,0.27f},{0.96f,0.12f},{0.45f,0.38f} };
        for (const auto& s : stars) fillRect(r, area.x + s[0] * area.w, area.y + s[1] * area.h, 2.0f, 2.0f);
    }

    // Sun or moon
    const float bodyX = area.x + kSkyBodies[index].x * area.w;
    const float bodyY = area.y + kSkyBodies[index].y * area.h;
    if (part == TimePart::Night) {
        SDL_SetRenderDrawColor(r, 232, 232, 214, 255);
        fillCircle(r, bodyX, bodyY, 17.0f);
        setColor(r, palette.sky);
        fillCircle(r, bodyX + 8.0f, bodyY - 4.0f, 15.0f);  // bite out of it: a crescent
    } else {
        SDL_SetRenderDrawColor(r, 255, 232, 140, 255);
        fillCircle(r, bodyX, bodyY, 21.0f);
    }

    // Hills, ground, road
    setColor(r, palette.hills);
    fillDome(r, area.x + area.w * 0.22f, horizon, 120.0f, 46.0f);
    fillDome(r, area.x + area.w * 0.70f, horizon, 150.0f, 58.0f);
    setColor(r, palette.ground);
    fillRect(r, area.x, horizon, area.w, area.y + area.h - horizon);
    const float roadY = horizon + (area.y + area.h - horizon) * 0.45f;
    setColor(r, palette.road);
    fillRect(r, area.x, roadY, area.w, (area.y + area.h - roadY) * 0.55f);

    // Wagon (bobs gently, wheels roll)
    const float groundY = roadY + (area.y + area.h - roadY) * 0.40f;
    const float bob = std::sin(seconds * 6.0f) * 1.5f;
    const float wagonX = area.x + area.w * 0.12f;
    const float wagonBottom = groundY - 14.0f + bob;

    setColor(r, kWagonWood);
    fillRect(r, wagonX, wagonBottom - 34.0f, 96.0f, 30.0f);                  // body
    setColor(r, kCanvas);
    fillDome(r, wagonX + 48.0f, wagonBottom - 34.0f, 48.0f, 36.0f);          // canvas cover
    setColor(r, kOutline);
    fillRect(r, wagonX + 96.0f, wagonBottom - 24.0f, 22.0f, 3.0f);           // hitch pole
    drawWheel(r, wagonX + 22.0f, groundY - 2.0f + bob, 16.0f, seconds * 3.0f);
    drawWheel(r, wagonX + 76.0f, groundY - 2.0f + bob, 16.0f, seconds * 3.0f);

    // Two oxen, watching
    const float stride = seconds * 6.0f;
    drawOx(r, wagonX + 112.0f, groundY + 2.0f, stride);
    drawOx(r, wagonX + 196.0f, groundY + 2.0f, stride + kPi);

    // Dread tints everything red as sanity drops
    if (dread > 0.0f) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 130, 0, 25, static_cast<Uint8>(std::min(1.0f, dread) * 85.0f));
        fillRect(r, area.x, area.y, area.w, area.h);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }

    SDL_SetRenderClipRect(r, nullptr);
    setColor(r, kOutline);
    SDL_RenderRect(r, &area);
}

void SceneRenderer::drawProgress(SDL_Renderer* r, const TextRenderer& text, const SDL_FRect& area,
                                 int totalDays, int currentDay, TimePart currentPart) const {
    const int total = std::max(1, totalDays) * kPartsPerDay;
    const int current = (std::max(1, currentDay) - 1) * kPartsPerDay + static_cast<int>(currentPart);
    const float margin = 14.0f;
    const float left = area.x + margin;
    const float right = area.x + area.w - margin;
    const float lineY = area.y + area.h - 18.0f;
    const float step = total > 1 ? (right - left) / static_cast<float>(total - 1) : 0.0f;

    // The trail itself
    SDL_SetRenderDrawColor(r, 120, 100, 70, 255);
    fillRect(r, left, lineY - 1.0f, right - left, 3.0f);

    for (int i = 0; i < total; ++i) {
        const float x = left + step * static_cast<float>(i);

        if (i % kPartsPerDay == 0) {  // a "Day N" label above each day's first dot
            SDL_SetRenderDrawColor(r, 60, 40, 30, 255);
            text.drawLine(r, "Day " + std::to_string(i / kPartsPerDay + 1), x - 6.0f, area.y, 2.0f);
        }

        if (i < current) {            // done
            SDL_SetRenderDrawColor(r, 90, 70, 50, 255);
            fillCircle(r, x, lineY, 6.0f);
        } else if (i == current) {    // you are here
            SDL_SetRenderDrawColor(r, 245, 200, 70, 255);
            fillCircle(r, x, lineY, 10.0f);
            SDL_SetRenderDrawColor(r, 60, 40, 30, 255);
            strokeCircle(r, x, lineY, 10.0f);
            strokeCircle(r, x, lineY, 9.0f);
        } else {                      // still to come
            SDL_SetRenderDrawColor(r, 232, 215, 175, 255);
            fillCircle(r, x, lineY, 6.0f);
            SDL_SetRenderDrawColor(r, 90, 70, 50, 255);
            strokeCircle(r, x, lineY, 6.0f);
        }
    }
}
