#include "SceneActors.h"

#include <cmath>
#include <string>

#include "DrawUtil.h"

using namespace draw;

namespace {
    const SDL_Color kOutline  {  50,  35,  25, 255 };
    const SDL_Color kSkin     { 238, 200, 162, 255 };
    const SDL_Color kPaCoat   {  86,  98, 126, 255 };
    const SDL_Color kHat      {  70,  50,  38, 255 };
    const SDL_Color kWood     { 110,  72,  45, 255 };
    const SDL_Color kCream    { 236, 226, 200, 255 };
    const SDL_Color kOxHide   {  92,  70,  58, 255 };
    const SDL_Color kDeerHide { 150, 104,  66, 255 };
    const SDL_Color kFlameA   { 245, 150,  40, 255 };
    const SDL_Color kFlameB   { 255, 220,  90, 255 };
    const SDL_Color kWater    {  70, 130, 190, 255 };
    const SDL_Color kGold     { 240, 190,  50, 255 };

    constexpr float kPersonScale = 1.45f;   // everybody is drawn this much bigger than their "scale" so faces and props read at a glance

    // ----- people ---------------------------------------------------------------------------------

    struct Person {
        float x = 0.0f;          // where the feet are (centre)
        float y = 0.0f;          // ground
        float scale = 1.0f;
        float dir = 1.0f;        // +1 faces right, -1 faces left
        float lean = 0.0f;       // lean forward, in "pixels" of shoulder offset (x scale)
        float armFront = 0.3f;   // arm angles: 0 = hanging down, +pi/2 = straight out ahead, pi = straight up
        float armBack = 0.2f;
        float legPhase = 0.0f;   // walking: radians; 0 with legSwing 0 = standing still
        float legSwing = 0.0f;   // how far the feet swing (0..1)
        float crouch = 0.0f;     // 0 standing, 1 crouching/kneeling
        float headDx = 0.0f;     // head wobble (shaking, looking about)
        SDL_Color coat = kPaCoat;
        SDL_Color hatColor = kHat;
        bool hat = true;
        bool eyes = true;
    };

    struct Hands {
        SDL_FPoint front{}, back{}, head{}, mouth{};
    };

    Hands drawPerson(const StageContext& c, const Person& p) {
        SDL_Renderer* r = c.r;
        const float s = p.scale * kPersonScale, d = p.dir;
        const float legLen = 24.0f * s * (1.0f - 0.45f * p.crouch);
        const float hipX = p.x, hipY = p.y - legLen;
        const float shX = hipX + d * p.lean * s;
        const float shY = hipY - 20.0f * s * (1.0f - 0.12f * p.crouch);
        const SDL_FPoint headC{ shX + d * 1.5f * s + p.headDx * s, shY - 9.0f * s };

        // legs
        const float swing = std::sin(p.legPhase) * 9.0f * s * p.legSwing;
        setColor(r, kOutline);
        const float stance = p.crouch > 0.5f ? 0.0f : 3.0f * s;      // feet a little apart when standing
        thickLine(r, hipX, hipY, hipX + d * (swing + stance), p.y, 3.0f * s);
        thickLine(r, hipX, hipY, hipX - d * (swing + stance), p.y, 3.0f * s);
        if (p.crouch > 0.5f) {                      // knees on the floor: feet stick out behind
            thickLine(r, hipX, p.y - 2.0f, hipX - d * 10.0f * s, p.y - 2.0f, 3.0f * s);
        }
        // torso
        setColor(r, p.coat);
        thickLine(r, hipX, hipY, shX, shY, 6.0f * s);
        // back arm (behind the body)
        const float armLen = 15.0f * s;
        Hands h;
        h.back  = { shX + d * std::sin(p.armBack)  * armLen, shY + std::cos(p.armBack)  * armLen };
        h.front = { shX + d * std::sin(p.armFront) * armLen, shY + std::cos(p.armFront) * armLen };
        setColor(r, p.coat);
        thickLine(r, shX, shY, h.back.x, h.back.y, 3.0f * s);
        // head
        setColor(r, kSkin);
        fillCircle(r, headC.x, headC.y, 7.5f * s);
        if (p.hat) {
            setColor(r, p.hatColor);
            fillRect(r, headC.x - 6.0f * s, headC.y - 15.0f * s, 12.0f * s, 9.0f * s);   // crown
            fillRect(r, headC.x - 11.0f * s, headC.y - 7.0f * s, 22.0f * s, 3.0f * s);   // brim
        }
        if (p.eyes) {
            setColor(r, kOutline);
            fillRect(r, headC.x + d * 3.0f * s - 1.0f, headC.y - 2.0f * s, 2.0f, 2.5f * s);
            fillRect(r, headC.x + d * 6.5f * s - 1.0f, headC.y - 2.0f * s, 2.0f, 2.5f * s);
        }
        h.mouth = { headC.x + d * 5.0f * s, headC.y + 3.5f * s };
        h.head = headC;
        // front arm (in front of the body)
        setColor(r, p.coat);
        thickLine(r, shX, shY, h.front.x, h.front.y, 3.0f * s);
        setColor(r, kSkin);
        fillCircle(r, h.front.x, h.front.y, 2.2f * s);
        fillCircle(r, h.back.x, h.back.y, 2.2f * s);
        return h;
    }

    void bubble(const StageContext& c, float cx, float bottomY, const std::string& text, float scale,
                SDL_Color fill = SDL_Color{ 255, 252, 240, 255 }) {
        const float w = c.text->textWidth(text, scale) + 14.0f;
        const float h = 8.0f * scale + 12.0f;
        float x = cx - w * 0.5f;
        x = std::max(c.area.x + 4.0f, std::min(x, c.area.x + c.area.w - w - 4.0f));
        const float y = std::max(c.area.y + 4.0f, bottomY - h - 8.0f);
        setColor(c.r, fill);
        fillRect(c.r, x, y, w, h);
        for (int i = 0; i < 8; ++i) fillRect(c.r, cx - 6.0f + static_cast<float>(i) * 0.75f, y + h + static_cast<float>(i), 12.0f - static_cast<float>(i) * 1.5f, 1.0f);
        setColor(c.r, kOutline);
        const SDL_FRect box{ x, y, w, h };
        draw::strokeRect(c.r, box);
        c.label(text, x + 7.0f, y + 6.0f, scale);
    }

    void flame(SDL_Renderer* r, float x, float baseY, float size, float t, float seed) {
        const float f1 = std::sin(t * 11.0f + seed) * 0.15f, f2 = std::sin(t * 7.0f + seed * 2.0f) * 0.2f;
        setColor(r, kFlameA);
        fillEllipse(r, x, baseY - size * (0.8f + f2), size * 0.55f, size * (0.9f + f2));
        setColor(r, kFlameB);
        fillEllipse(r, x + f1 * size, baseY - size * (0.55f + f2), size * 0.3f, size * (0.55f + f2));
    }

    void campfire(const StageContext& c, float x, float gy, float size) {
        SDL_Renderer* r = c.r;
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);                       // warm glow on the ground
        SDL_SetRenderDrawColor(r, 255, 170, 60, static_cast<Uint8>(46 + 14 * std::sin(c.t * 9.0f)));
        fillEllipse(r, x, gy - 2.0f, size * 3.0f, size * 0.8f);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        setColor(r, kWood);
        thickLine(r, x - size * 1.3f, gy - 2.0f, x + size * 1.1f, gy - 7.0f, 4.0f);
        thickLine(r, x + size * 1.3f, gy - 2.0f, x - size * 1.1f, gy - 7.0f, 4.0f);
        flame(r, x - size * 0.5f, gy - 5.0f, size * 0.9f, c.t, 1.0f);
        flame(r, x + size * 0.5f, gy - 5.0f, size * 0.8f, c.t, 2.3f);
        flame(r, x, gy - 5.0f, size * 1.2f, c.t, 4.1f);
    }

    // ----- animals ---------------------------------------------------------------------------------

    void drawDeer(const StageContext& c, float x, float gy, float s, bool hat, float phase) {
        SDL_Renderer* r = c.r;
        const float bob = std::sin(c.t * 0.8f + phase) * 1.2f * s;
        setColor(r, kDeerHide);
        for (float lx : { 10.0f, 18.0f, 40.0f, 48.0f }) fillRect(r, x + lx * s, gy - 34.0f * s, 4.0f * s, 34.0f * s);   // legs
        fillEllipse(r, x + 30.0f * s, gy - 40.0f * s, 31.0f * s, 13.0f * s);                                          // body
        thickLine(r, x + 8.0f * s, gy - 46.0f * s, x - 6.0f * s, gy - 72.0f * s + bob, 9.0f * s);                       // neck
        fillEllipse(r, x - 10.0f * s, gy - 76.0f * s + bob, 10.0f * s, 6.5f * s);                                      // head
        fillEllipse(r, x - 3.0f * s, gy - 84.0f * s + bob, 3.0f * s, 5.0f * s);                                        // ears
        fillEllipse(r, x + 3.0f * s, gy - 82.0f * s + bob, 3.0f * s, 5.0f * s);
        const float flick = std::sin(c.t * 4.0f + phase) > 0.92f ? -6.0f * s : 0.0f;                                    // tail flick
        setColor(r, kCream);
        fillEllipse(r, x + 61.0f * s, gy - 47.0f * s + flick, 4.0f * s, 6.0f * s);
        setColor(r, kOutline);                                                                                          // antlers
        thickLine(r, x - 6.0f * s, gy - 82.0f * s + bob, x - 9.0f * s, gy - 102.0f * s + bob, 2.0f);
        thickLine(r, x - 9.0f * s, gy - 94.0f * s + bob, x - 17.0f * s, gy - 100.0f * s + bob, 2.0f);
        thickLine(r, x - 8.0f * s, gy - 100.0f * s + bob, x - 2.0f * s, gy - 108.0f * s + bob, 2.0f);
        thickLine(r, x - 3.0f * s, gy - 82.0f * s + bob, x + 2.0f * s, gy - 100.0f * s + bob, 2.0f);
        SDL_SetRenderDrawColor(r, 250, 250, 250, 255);                                                                  // a calm, direct stare
        fillCircle(r, x - 14.0f * s, gy - 77.0f * s + bob, 2.8f * s);
        setColor(r, kOutline);
        fillCircle(r, x - 15.0f * s, gy - 77.0f * s + bob, 1.3f * s);
        fillRect(r, x - 20.0f * s, gy - 73.0f * s + bob, 3.0f * s, 2.0f * s);                                           // nose
        if (hat) {                                                                                                      // your hat
            setColor(r, kHat);
            fillRect(r, x - 14.0f * s, gy - 94.0f * s + bob, 11.0f * s, 8.0f * s);
            fillRect(r, x - 19.0f * s, gy - 87.0f * s + bob, 21.0f * s, 3.0f * s);
        }
    }

    // ----- scene actors ----------------------------------------------------------------------------

    void actorOxen(const StageContext& c, float blinkSpeed) {
        const float blink = std::fmod(c.t * blinkSpeed, 3.4f) > 3.2f ? 1.0f : 0.0f;
        stage::drawOx(c, c.zoneX(0.55f), c.groundY + 2.0f, 0.0f, blink);
        stage::drawOx(c, c.zoneX(0.55f) + 88.0f, c.groundY + 2.0f, 0.0f, blink);
    }

    void actorMapMan(const StageContext& c) {
        SDL_Renderer* r = c.r;
        const float gy = c.groundY, x = c.zoneX(0.60f);
        setColor(r, kWood);                                                       // stall
        fillRect(r, x, gy - 30.0f, 78.0f, 6.0f);
        fillRect(r, x + 4.0f, gy - 24.0f, 5.0f, 24.0f);
        fillRect(r, x + 69.0f, gy - 24.0f, 5.0f, 24.0f);
        for (int i = 0; i < 3; ++i) {                                              // rolled maps on the table
            const float mx = x + 10.0f + static_cast<float>(i) * 22.0f;
            setColor(r, kCream);
            fillRect(r, mx, gy - 38.0f, 18.0f, 8.0f);
            setColor(r, SDL_Color{ 190, 80, 60, 255 });
            fillRect(r, mx, gy - 36.0f, 18.0f, 2.0f);
        }
        Person man; man.x = x + 100.0f; man.y = gy; man.dir = -1.0f; man.coat = SDL_Color{ 150, 70, 60, 255 };
        man.hatColor = SDL_Color{ 60, 60, 70, 255 };
        man.armFront = kPi * (0.78f + 0.06f * std::sin(c.t * 5.0f));
        const Hands h = drawPerson(c, man);
        setColor(r, kCream);                                                      // the map he is waving, with a red X
        fillRect(r, h.front.x - 11.0f, h.front.y - 18.0f, 22.0f, 17.0f);
        setColor(r, SDL_Color{ 190, 40, 40, 255 });
        thickLine(r, h.front.x - 6.0f, h.front.y - 15.0f, h.front.x + 6.0f, h.front.y - 4.0f, 2.0f);
        thickLine(r, h.front.x + 6.0f, h.front.y - 15.0f, h.front.x - 6.0f, h.front.y - 4.0f, 2.0f);
        if (std::fmod(c.t, 2.4f) < 1.6f) bubble(c, x + 70.0f, gy - 82.0f, "MAPS!", 2.0f);
    }

    void actorDeer(const StageContext& c) { drawDeer(c, c.zoneX(0.62f), c.groundY, 1.0f, false, 0.0f); }

    void actorHerd(const StageContext& c) {
        drawDeer(c, c.zoneX(0.80f), c.groundY - 4.0f, 0.62f, false, 1.7f);
        drawDeer(c, c.zoneX(0.53f), c.groundY - 2.0f, 0.70f, false, 0.6f);
        drawDeer(c, c.zoneX(0.64f), c.groundY + 2.0f, 0.82f, true, 0.0f);          // the one in your hat
    }

    void actorRitual(const StageContext& c) {
        SDL_Renderer* r = c.r;
        const float cx = c.zoneX(0.76f), cy = c.groundY - 8.0f, rx = 82.0f, ry = 17.0f;
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 200, 20, 40, static_cast<Uint8>(55 + 30 * std::sin(c.t * 3.0f)));
        fillEllipse(r, cx, cy, rx, ry);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(r, 150, 20, 30, 255);
        strokeEllipse(r, cx, cy, rx, ry);
        strokeEllipse(r, cx, cy, rx - 1.0f, ry - 1.0f);
        SDL_FPoint pts[5];
        for (int i = 0; i < 5; ++i) {
            const float a = -kPi * 0.5f + 2.0f * kPi * static_cast<float>(i) / 5.0f;
            pts[i] = { cx + std::cos(a) * rx * 0.9f, cy + std::sin(a) * ry * 0.9f };
        }
        for (int i = 0; i < 5; ++i) draw::line(r, pts[i].x, pts[i].y, pts[(i + 2) % 5].x, pts[(i + 2) % 5].y);
        Person kid; kid.x = cx; kid.y = cy + 6.0f; kid.scale = 0.72f; kid.hat = false; kid.coat = SDL_Color{ 150, 120, 90, 255 };
        kid.armFront = kPi * (0.85f + 0.08f * std::sin(c.t * 4.0f)); kid.armBack = kPi * (0.85f - 0.08f * std::sin(c.t * 4.0f));
        drawPerson(c, kid);
        for (int i = 0; i < 5; ++i) {                                              // candles
            setColor(r, kCream);
            fillRect(r, pts[i].x - 2.0f, pts[i].y - 9.0f, 4.0f, 9.0f);
            flame(r, pts[i].x, pts[i].y - 9.0f, 4.5f, c.t, static_cast<float>(i) * 1.7f);
        }
    }

    void actorFigure(const StageContext& c, float x, float scale, float seed) {
        SDL_Renderer* r = c.r;
        Person f; f.x = x; f.y = c.groundY; f.scale = scale; f.dir = -1.0f;
        f.coat = SDL_Color{ 36, 40, 56, 255 }; f.hatColor = SDL_Color{ 30, 26, 24, 255 }; f.eyes = false;
        f.armFront = 0.08f; f.armBack = 0.08f;
        const Hands h = drawPerson(c, f);
        setColor(r, kPaCoat);                                                      // your good coat: a lighter collar
        fillRect(r, h.head.x - 5.0f * scale, h.head.y + 7.0f * scale, 10.0f * scale, 5.0f * scale);
        const bool on = std::fmod(c.t + seed, 4.0f) < 3.6f;                        // two pale eyes, rarely closing
        if (on) {
            SDL_SetRenderDrawColor(r, 250, 250, 210, 255);
            fillRect(r, h.head.x - 4.0f * scale, h.head.y - 2.0f * scale, 2.5f * scale, 3.0f * scale);
            fillRect(r, h.head.x + 1.5f * scale, h.head.y - 2.0f * scale, 2.5f * scale, 3.0f * scale);
        }
    }

    void actorFigureScene(const StageContext& c) {
        campfire(c, c.zoneX(0.60f), c.groundY, 12.0f);
        actorFigure(c, c.zoneX(0.90f), 1.15f, 0.0f);
    }

    void actorCrowd(const StageContext& c) {
        campfire(c, c.zoneX(0.57f), c.groundY, 10.0f);
        const float xs[5] = { 0.70f, 0.78f, 0.86f, 0.94f, 0.74f };
        const float sc[5] = { 1.0f, 0.95f, 1.1f, 1.0f, 0.8f };
        for (int i = 4; i >= 0; --i) {
            StageContext shifted = c;
            shifted.groundY = c.groundY - (i == 4 ? 14.0f : 0.0f);
            actorFigure(shifted, c.zoneX(xs[i]), sc[i], static_cast<float>(i) * 0.7f);
        }
    }

    void actorTidyWagon(const StageContext& c) {
        SDL_Renderer* r = c.r;
        const char* labels[6] = { "FOOD", "TOOLS", "OATS", "SOCKS", "???", "MINE" };
        const float x0 = c.zoneX(0.58f), gy = c.groundY;
        const float bw = 50.0f, bh = 26.0f;
        const struct { float col, row; } slots[6] = { {0, 0}, {1, 0}, {2, 0}, {0.5f, 1}, {1.5f, 1}, {1.0f, 2} };
        for (int i = 0; i < 6; ++i) {
            const float bx = x0 + slots[i].col * (bw + 2.0f), by = gy - bh - slots[i].row * (bh + 2.0f);
            setColor(r, kWood);
            fillRect(r, bx, by, bw, bh);
            setColor(r, kCream);
            fillRect(r, bx + 4.0f, by + 7.0f, bw - 8.0f, 12.0f);                    // label
            setColor(r, kOutline);
            const SDL_FRect box{ bx, by, bw, bh };
            draw::strokeRect(r, box);
            c.label(labels[i], bx + (bw - c.text->textWidth(labels[i], 1.0f)) * 0.5f, by + 11.0f, 1.0f);
        }
        for (int i = 0; i < 4; ++i) {                                              // tidy sparkles
            const float ph = std::fmod(c.t * 0.9f + static_cast<float>(i) * 0.27f, 1.0f);
            const float sx = x0 + 12.0f + static_cast<float>(i) * 41.0f, sy = gy - 100.0f + static_cast<float>(i % 2) * 22.0f;
            SDL_SetRenderDrawColor(r, 255, 250, 200, 255);
            starBurst(r, sx, sy, 1.0f, 3.0f + 6.0f * std::sin(ph * kPi), 4, ph);
        }
    }

    void actorRiver(const StageContext& c) {
        SDL_Renderer* r = c.r;
        const float gy = c.groundY;
        const float signX = c.zoneX(0.50f);
        setColor(r, kWood);                                                        // two signs on one post
        fillRect(r, signX + 38.0f, gy - 78.0f, 4.0f, 80.0f);
        SDL_SetRenderDrawColor(r, 90, 160, 90, 255);
        fillRect(r, signX - 4.0f, gy - 84.0f, 92.0f, 18.0f);
        SDL_SetRenderDrawColor(r, 250, 250, 240, 255);
        c.label("SHALLOW", signX + 2.0f, gy - 79.0f, 1.0f + 0.0f);
        SDL_SetRenderDrawColor(r, 230, 220, 190, 255);
        fillRect(r, signX + 6.0f, gy - 58.0f, 70.0f, 7.0f);
        SDL_SetRenderDrawColor(r, 60, 40, 30, 255);
        c.label("NOT ANYMORE", signX + 8.0f, gy - 57.0f, 0.75f);
        // the water, rising and falling
        const float level = 16.0f + 7.0f * std::sin(c.t * 0.9f);
        const float left = c.zoneX(0.60f), right = c.area.x + c.area.w;
        for (float x = left; x < right; x += 2.0f) {
            const float wave = std::sin(x * 0.09f + c.t * 3.0f) * 2.5f;
            setColor(r, kWater);
            fillRect(r, x, gy - level + wave, 2.0f, level + 14.0f - wave);
            SDL_SetRenderDrawColor(r, 150, 200, 235, 255);
            fillRect(r, x, gy - level + wave, 2.0f, 2.0f);
        }
    }

    void actorTravellers(const StageContext& c) {
        SDL_Renderer* r = c.r;
        const float gy = c.groundY;
        SDL_SetRenderDrawColor(r, 190, 70, 70, 255);                               // picnic blanket
        fillRect(r, c.zoneX(0.56f), gy - 6.0f, c.area.w * 0.42f, 6.0f);
        SDL_SetRenderDrawColor(r, 240, 235, 225, 255);
        for (float bx = c.zoneX(0.56f); bx < c.zoneX(0.98f); bx += 12.0f) fillRect(r, bx, gy - 6.0f, 6.0f, 6.0f);
        const float sway = std::sin(c.t * 2.0f);
        for (int i = 0; i < 3; ++i) {
            Person p; p.x = c.zoneX(0.64f + 0.14f * static_cast<float>(i)); p.y = gy - 4.0f; p.scale = 1.05f;
            p.dir = -1.0f; p.lean = sway * 2.5f; p.headDx = sway * 1.5f; p.hat = i != 1;
            p.coat = i == 0 ? SDL_Color{ 120, 150, 110, 255 } : i == 1 ? SDL_Color{ 170, 130, 90, 255 } : SDL_Color{ 130, 120, 160, 255 };
            p.armFront = 0.5f + 0.3f * sway; p.armBack = 0.4f - 0.3f * sway;
            const Hands h = drawPerson(c, p);
            const float smile = 5.0f + 2.5f * std::sin(c.t * 2.0f);                // all smiling at exactly the same speed
            setColor(r, kOutline);
            for (float dx = -smile; dx <= smile; dx += 1.0f) {
                fillRect(r, h.head.x + dx - 1.0f * p.dir, h.head.y + 7.0f - (dx * dx) / (smile * 2.0f), 1.5f, 1.5f);
            }
        }
    }

    void actorOxenCircle(const StageContext& c) {
        SDL_Renderer* r = c.r;
        const float cx = c.zoneX(0.77f), cy = c.groundY - 16.0f, rx = 70.0f, ry = 16.0f;
        // a flip chart in the middle of the circle
        setColor(r, kWood);
        thickLine(r, cx - 8.0f, cy + 14.0f, cx - 2.0f, cy - 24.0f, 2.0f);
        thickLine(r, cx + 8.0f, cy + 14.0f, cx + 2.0f, cy - 24.0f, 2.0f);
        setColor(r, kCream);
        fillRect(r, cx - 14.0f, cy - 44.0f, 28.0f, 22.0f);
        SDL_SetRenderDrawColor(r, 60, 40, 30, 255);
        draw::line(r, cx - 10.0f, cy - 38.0f, cx + 10.0f, cy - 38.0f);
        draw::line(r, cx - 10.0f, cy - 33.0f, cx + 6.0f, cy - 33.0f);
        draw::line(r, cx - 10.0f, cy - 28.0f, cx + 9.0f, cy - 28.0f);
        const int n = 6;
        for (int i = 0; i < n; ++i) {
            const float a = c.t * 0.35f + 2.0f * kPi * static_cast<float>(i) / n;
            const float ox = cx + std::cos(a) * rx, oy = cy + std::sin(a) * ry;
            setColor(r, kOxHide);
            fillEllipse(r, ox, oy, 17.0f, 7.0f);
            fillCircle(r, ox - std::cos(a) * 15.0f, oy - std::sin(a) * 5.0f, 5.0f);   // heads face inward
            SDL_SetRenderDrawColor(r, 245, 245, 245, 255);
            fillRect(r, ox - std::cos(a) * 17.0f - 1.0f, oy - std::sin(a) * 6.0f - 2.0f, 2.0f, 2.0f);
        }
    }

    void actorVoice(const StageContext& c) {
        SDL_Renderer* r = c.r;
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);                        // it is dark out
        SDL_SetRenderDrawColor(r, 5, 5, 25, 105);
        fillRect(r, c.area.x, c.area.y, c.area.w, c.area.h);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        Person me; me.x = c.zoneX(0.82f); me.y = c.groundY; me.dir = -1.0f; me.eyes = false;
        me.coat = SDL_Color{ 14, 16, 30, 255 }; me.hatColor = SDL_Color{ 10, 10, 18, 255 };
        me.armFront = 0.1f; me.armBack = 0.1f;
        const Hands h = drawPerson(c, me);
        SDL_SetRenderDrawColor(r, 250, 250, 220, 255);
        fillRect(r, h.head.x - 4.0f, h.head.y - 2.0f, 3.0f, 3.0f);
        fillRect(r, h.head.x + 1.0f, h.head.y - 2.0f, 3.0f, 3.0f);
        const float wob = std::sin(c.t * 3.0f) * 2.0f;
        bubble(c, c.zoneX(0.68f) + wob, c.groundY - 62.0f, "BORROW?", 2.0f, SDL_Color{ 220, 225, 245, 255 });
    }

    void actorNewRoad(const StageContext& c) {
        SDL_Renderer* r = c.r;
        const float left = c.zoneX(0.38f), right = c.area.x + c.area.w;
        const float top = c.roadY - 2.0f, bottom = c.area.y + c.area.h;
        SDL_SetRenderDrawColor(r, 214, 216, 224, 255);                             // smooth, pale, clearly meant for you
        fillRect(r, left, top, right - left, bottom - top);
        SDL_SetRenderDrawColor(r, 240, 242, 250, 255);
        fillRect(r, left, top, right - left, 2.0f);
        SDL_SetRenderDrawColor(r, 120, 150, 215, 255);                             // chevrons that slide along it, in one direction
        const float shift = std::fmod(c.t * 60.0f, 44.0f);
        for (float x = left - 44.0f + shift; x < right; x += 44.0f) {
            const float cy = (top + bottom) * 0.5f;
            if (x < left) continue;
            thickLine(c.r, x, cy - 9.0f, x + 12.0f, cy, 3.0f);
            thickLine(c.r, x + 12.0f, cy, x, cy + 9.0f, 3.0f);
        }
        setColor(r, kWood);
        fillRect(r, c.zoneX(0.80f), c.groundY - 70.0f, 4.0f, 72.0f);
        SDL_SetRenderDrawColor(r, 235, 235, 245, 255);
        fillRect(r, c.zoneX(0.72f), c.groundY - 78.0f, 80.0f, 18.0f);
        SDL_SetRenderDrawColor(r, 60, 40, 30, 255);
        c.label("THIS WAY", c.zoneX(0.72f) + 8.0f, c.groundY - 73.0f, 1.0f);
    }

    void actorCookCamp(const StageContext& c) {
        SDL_Renderer* r = c.r;
        const float gy = c.groundY, fx = c.zoneX(0.62f);
        campfire(c, fx, gy, 11.0f);
        setColor(r, kOutline);                                                     // tripod and pot
        thickLine(r, fx - 20.0f, gy, fx, gy - 44.0f, 2.0f);
        thickLine(r, fx + 20.0f, gy, fx, gy - 44.0f, 2.0f);
        SDL_SetRenderDrawColor(r, 40, 40, 46, 255);
        fillEllipse(r, fx, gy - 30.0f, 16.0f, 11.0f);
        SDL_SetRenderDrawColor(r, 90, 70, 50, 255);
        fillEllipse(r, fx, gy - 38.0f, 15.0f, 3.5f);
        for (int i = 0; i < 5; ++i) {                                              // the smell, drifting toward your wagon
            const float ph = std::fmod(c.t * 0.5f + static_cast<float>(i) * 0.2f, 1.0f);
            const float sx = fx - ph * 90.0f, sy = gy - 50.0f - ph * 22.0f + std::sin(ph * 12.0f + static_cast<float>(i)) * 4.0f;
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 255, 255, 255, static_cast<Uint8>(170 * (1.0f - ph)));
            fillCircle(r, sx, sy, 3.0f + ph * 3.0f);
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        }
        Person cook; cook.x = c.zoneX(0.88f); cook.y = gy; cook.dir = -1.0f; cook.scale = 1.1f;
        cook.coat = SDL_Color{ 235, 235, 230, 255 }; cook.hatColor = SDL_Color{ 245, 245, 245, 255 };
        cook.armFront = kPi * (0.8f + 0.1f * std::sin(c.t * 5.0f)); cook.armBack = 0.5f;
        drawPerson(c, cook);
    }

    // ----- actions: Pa walks out and does the thing --------------------------------------------------

    void rayBurst(const StageContext& c, float cx, float cy, float t) {
        SDL_SetRenderDrawBlendMode(c.r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(c.r, 255, 240, 160, 150);
        for (int i = 0; i < 9; ++i) {
            const float a = t * 0.4f + 2.0f * kPi * static_cast<float>(i) / 9.0f;
            thickLine(c.r, cx + std::cos(a) * 14.0f, cy + std::sin(a) * 14.0f, cx + std::cos(a) * 46.0f, cy + std::sin(a) * 46.0f, 3.0f);
        }
        SDL_SetRenderDrawBlendMode(c.r, SDL_BLENDMODE_NONE);
    }

    void heart(SDL_Renderer* r, float x, float y, float s) {
        SDL_SetRenderDrawColor(r, 225, 60, 90, 255);
        fillCircle(r, x - s * 0.5f, y, s * 0.55f);
        fillCircle(r, x + s * 0.5f, y, s * 0.55f);
        for (float i = 0.0f; i < s * 1.1f; i += 1.0f) fillRect(r, x - s * 1.05f + i, y + i * 0.55f, s * 2.1f - 2.0f * i, 1.0f);
    }
}

// ============================================================================================
namespace stage {

void drawOx(const StageContext& c, float x, float groundY, float legPhase, float blink) {
    SDL_Renderer* r = c.r;
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
    draw::line(r, x + 58.0f, groundY - 52.0f, x + 54.0f, groundY - 62.0f);  // horns
    draw::line(r, x + 72.0f, groundY - 52.0f, x + 76.0f, groundY - 62.0f);
    SDL_SetRenderDrawColor(r, 245, 245, 245, 255);
    fillRect(r, x + 58.0f, groundY - 48.0f, 5.0f, 5.0f);   // eyes
    fillRect(r, x + 66.0f, groundY - 48.0f, 5.0f, 5.0f);
    setColor(r, kOutline);
    fillRect(r, x + 60.0f, groundY - 46.0f, 2.0f, 2.0f);   // pupils, fixed on the viewer
    fillRect(r, x + 66.0f, groundY - 46.0f, 2.0f, 2.0f);
    if (blink > 0.5f) {                                    // eyelids
        setColor(r, kOxHide);
        fillRect(r, x + 58.0f, groundY - 48.0f, 5.0f, 5.0f);
        fillRect(r, x + 66.0f, groundY - 48.0f, 5.0f, 5.0f);
        setColor(r, kOutline);
        fillRect(r, x + 58.0f, groundY - 46.0f, 5.0f, 1.0f);
        fillRect(r, x + 66.0f, groundY - 46.0f, 5.0f, 1.0f);
    }
}

void drawSceneActor(const StageContext& c, SceneId scene) {
    switch (scene) {
    case SceneId::Trail:       break;                                   // the wagon and oxen are drawn by the scene itself
    case SceneId::OxenStare:   actorOxen(c, 1.0f);       break;
    case SceneId::MapMan:      actorMapMan(c);           break;
    case SceneId::Deer:        actorDeer(c);             break;
    case SceneId::Herd:        actorHerd(c);             break;
    case SceneId::Ritual:      actorRitual(c);           break;
    case SceneId::Figure:      actorFigureScene(c);      break;
    case SceneId::Crowd:       actorCrowd(c);            break;
    case SceneId::TidyWagon:   actorTidyWagon(c);        break;
    case SceneId::River:       actorRiver(c);            break;
    case SceneId::Travellers:  actorTravellers(c);       break;
    case SceneId::OxenCircle:  actorOxenCircle(c);       break;
    case SceneId::Voice:       actorVoice(c);            break;
    case SceneId::NewRoad:     actorNewRoad(c);          break;
    case SceneId::CookCamp:    actorCookCamp(c);         break;
    case SceneId::Count:       break;
    }
}

void drawAction(const StageContext& c, ActionId action, float seconds) {
    if (action == ActionId::None) return;
    SDL_Renderer* r = c.r;
    const float gy = c.groundY;
    const float homeX = c.wagonX + 122.0f;                // just beyond the wagon's hitch
    float targetX = c.zoneX(0.42f);
    if (action == ActionId::Join) targetX = c.zoneX(0.50f);
    const float arrive = clamp01(seconds / 0.8f);
    const float t = std::max(0.0f, seconds - 0.8f);      // time spent doing the thing
    const float walkX = lerp(homeX, targetX, arrive);
    const bool walking = arrive < 1.0f;

    Person pa; pa.x = walkX; pa.y = gy; pa.scale = 1.15f;
    if (walking) { pa.legPhase = seconds * 14.0f; pa.legSwing = 1.0f; pa.armFront = 0.5f * std::sin(seconds * 14.0f); pa.armBack = -pa.armFront; }

    // For props that need Pa to stand still and animate in place, 'pa' is filled in per action.
    switch (action) {
    case ActionId::Feed: {
        pa.armFront = kPi * 0.5f;
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        SDL_SetRenderDrawColor(r, 150, 110, 60, 255);                       // a bucket of oats
        fillRect(r, h.front.x - 6.0f, h.front.y - 2.0f, 13.0f, 9.0f);
        for (int k = 0; k < 7; ++k) {
            const float ph = fract(t * 1.1f + static_cast<float>(k) / 7.0f);
            setColor(r, kGold);
            fillCircle(r, h.front.x + 4.0f + ph * 54.0f, h.front.y + 4.0f + ph * ph * (gy - h.front.y - 4.0f), 2.0f);
        }
        break; }
    case ActionId::Eat: {
        pa.armBack = kPi * 0.42f;
        const float bite = 0.5f + 0.5f * std::sin(t * 5.0f);
        pa.armFront = lerp(kPi * 0.45f, kPi * 0.95f, bite);
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        SDL_SetRenderDrawColor(r, 235, 225, 200, 255);                      // the bowl, steaming
        fillEllipse(r, h.back.x + 5.0f, h.back.y + 3.0f, 8.0f, 5.0f);
        SDL_SetRenderDrawColor(r, 150, 80, 40, 255);
        fillEllipse(r, h.back.x + 5.0f, h.back.y, 6.5f, 2.5f);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        for (int k = 0; k < 3; ++k) {
            const float ph = fract(t * 0.8f + static_cast<float>(k) * 0.33f);
            SDL_SetRenderDrawColor(r, 255, 255, 255, static_cast<Uint8>(190 * (1.0f - ph)));
            fillCircle(r, h.back.x + 5.0f + std::sin(ph * 9.0f + static_cast<float>(k)) * 3.0f, h.back.y - 6.0f - ph * 24.0f, 2.5f + ph * 2.0f);
        }
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        setColor(r, kOutline);                                              // spoon
        thickLine(r, h.front.x, h.front.y, h.front.x + 7.0f, h.front.y - 5.0f, 2.0f);
        if (bite > 0.8f) { setColor(r, kOutline); fillRect(r, h.mouth.x - 1.0f, h.mouth.y - 1.0f, 4.0f, 4.0f); }
        break; }
    case ActionId::Rush: {
        pa.legPhase = seconds * 22.0f; pa.legSwing = 1.0f; pa.lean = 5.0f;
        pa.armFront = std::sin(seconds * 22.0f) * 1.1f; pa.armBack = -pa.armFront;
        pa.x = lerp(homeX, targetX, 1.0f);
        for (int k = 0; k < 8; ++k) {                                       // speed streaks
            const float ph = fract(seconds * 1.7f + static_cast<float>(k) * 0.125f);
            const float sy = c.roadY + 3.0f + static_cast<float>(k % 4) * 8.0f;
            SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
            const float sx = c.area.x + c.area.w - ph * (c.area.w + 40.0f);
            draw::line(r, sx, sy, sx + 34.0f, sy);
        }
        drawPerson(c, pa);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);                 // dust kicked up behind
        for (int k = 0; k < 4; ++k) {
            const float ph = fract(seconds * 3.0f + static_cast<float>(k) * 0.25f);
            SDL_SetRenderDrawColor(r, 200, 180, 140, static_cast<Uint8>(200 * (1.0f - ph)));
            fillCircle(r, pa.x - 8.0f - ph * 26.0f, gy - 3.0f - ph * 8.0f, 3.0f + ph * 4.0f);
        }
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        break; }
    case ActionId::Ask: {
        pa.armBack = kPi * 0.7f; pa.headDx = 1.5f * std::sin(t * 2.0f);
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        const float pulse = 2.5f + 0.4f * std::sin(t * 6.0f);
        bubble(c, h.head.x + 4.0f, h.head.y - 12.0f, "?", pulse);
        break; }
    case ActionId::Buy: {
        pa.armFront = kPi * 0.5f;
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        const float ph = fract(t * 0.8f);
        const float coinX = h.front.x + ph * 60.0f, coinY = h.front.y - std::sin(ph * kPi) * 22.0f;
        setColor(r, kGold);
        fillCircle(r, coinX, coinY, 4.0f);
        SDL_SetRenderDrawColor(r, 200, 150, 30, 255);
        strokeCircle(r, coinX, coinY, 4.0f);
        const float back = fract(t * 0.8f + 0.5f);                          // and something comes back the other way
        const float px = h.front.x + 60.0f - back * 56.0f, py = h.front.y - std::sin(back * kPi) * 14.0f;
        setColor(r, kCream);
        fillRect(r, px - 7.0f, py - 5.0f, 14.0f, 10.0f);
        SDL_SetRenderDrawColor(r, 190, 60, 50, 255);
        fillRect(r, px - 7.0f, py - 1.0f, 14.0f, 2.0f);
        break; }
    case ActionId::Refuse: {
        pa.armFront = kPi * 0.5f; pa.headDx = 2.0f * std::sin(t * 9.0f);    // an outstretched palm and a shaken head
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        fillRect(r, h.front.x - 1.0f, h.front.y - 6.0f, 4.0f, 12.0f);
        const float size = 15.0f + 2.0f * std::sin(t * 8.0f);
        SDL_SetRenderDrawColor(r, 205, 40, 40, 255);                        // a big "no"
        const float nx = h.head.x + 8.0f, ny = h.head.y - 30.0f;
        for (float k = 0.0f; k < 3.0f; k += 1.0f) strokeCircle(r, nx, ny, size - k);
        thickLine(r, nx - size * 0.7f, ny + size * 0.7f, nx + size * 0.7f, ny - size * 0.7f, 4.0f);
        break; }
    case ActionId::Flee: {
        const float lap = fract(seconds * 0.7f);
        pa.dir = -1.0f; pa.lean = 5.0f;
        pa.x = lerp(targetX + 30.0f, c.wagonX + 126.0f, lap);
        pa.legPhase = seconds * 24.0f; pa.legSwing = 1.0f;
        pa.armFront = std::sin(seconds * 24.0f) * 1.2f; pa.armBack = -pa.armFront;
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        for (int k = 0; k < 5; ++k) {
            const float ph = fract(seconds * 3.0f + static_cast<float>(k) * 0.2f);
            SDL_SetRenderDrawColor(r, 200, 180, 140, static_cast<Uint8>(210 * (1.0f - ph)));
            fillCircle(r, pa.x + 8.0f + ph * 30.0f, gy - 3.0f - ph * 9.0f, 3.0f + ph * 4.0f);
        }
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        const Hands h = drawPerson(c, pa);
        bubble(c, h.head.x, h.head.y - 16.0f, "AAH!", 1.5f, SDL_Color{ 255, 230, 160, 255 });
        break; }
    case ActionId::Fight: {
        const float swing = std::sin(t * 7.0f);
        pa.armFront = kPi * (0.55f + 0.35f * swing); pa.armBack = 0.3f; pa.lean = 2.0f + 2.0f * swing;
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        setColor(r, kWood);                                                 // a good stout stick
        thickLine(r, h.front.x, h.front.y, h.front.x + 12.0f, h.front.y - 22.0f * (0.4f + 0.6f * (swing * 0.5f + 0.5f)), 3.0f);
        if (swing > 0.55f) {
            const float ix = h.front.x + 30.0f, iy = h.front.y + 2.0f;
            SDL_SetRenderDrawColor(r, 255, 230, 80, 255);
            starBurst(r, ix, iy, 4.0f, 16.0f, 10, t * 3.0f);
            SDL_SetRenderDrawColor(r, 255, 120, 40, 255);
            c.label("POW", ix - 12.0f, iy - 28.0f, 2.0f);
        }
        break; }
    case ActionId::Hide: {
        pa.crouch = 1.0f; pa.eyes = true; pa.headDx = 1.8f * std::sin(t * 1.6f);
        pa.x = targetX;
        drawPerson(c, pa);
        if (walking) break;
        SDL_SetRenderDrawColor(r, 60, 120, 60, 255);                        // a bush, with Pa behind it
        fillDome(r, pa.x + 4.0f, gy + 1.0f, 26.0f, 24.0f);
        fillDome(r, pa.x - 16.0f, gy + 1.0f, 18.0f, 18.0f);
        fillDome(r, pa.x + 24.0f, gy + 1.0f, 18.0f, 18.0f);
        SDL_SetRenderDrawColor(r, 40, 90, 45, 255);
        for (int k = 0; k < 6; ++k) fillCircle(r, pa.x - 14.0f + static_cast<float>(k) * 7.0f, gy - 7.0f - static_cast<float>(k % 3) * 4.0f, 2.0f);
        break; }
    case ActionId::Pray: {
        pa.crouch = 1.0f; pa.armFront = kPi * 0.42f; pa.armBack = kPi * 0.42f; pa.lean = 2.0f;
        pa.x = targetX;
        if (!walking) rayBurst(c, pa.x, gy - 62.0f, t);
        const Hands h = drawPerson(c, pa);
        if (!walking) {
            SDL_SetRenderDrawColor(r, 255, 225, 120, 255);
            strokeEllipse(r, h.head.x, h.head.y - 18.0f + std::sin(t * 3.0f) * 1.5f, 10.0f, 3.0f);
            strokeEllipse(r, h.head.x, h.head.y - 18.0f + std::sin(t * 3.0f) * 1.5f, 10.0f, 2.0f);
        }
        break; }
    case ActionId::Steal: {
        pa.lean = 7.0f; pa.crouch = 0.35f;
        pa.legPhase = t * 4.0f; pa.legSwing = 0.5f; pa.headDx = 2.5f * std::sin(t * 3.0f);   // eyes everywhere
        pa.armFront = kPi * 0.6f; pa.armBack = 0.5f;
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        SDL_SetRenderDrawColor(r, 150, 110, 70, 255);                       // a sack, bouncing on the back
        const float hop = std::fabs(std::sin(t * 4.0f)) * 2.0f;
        fillCircle(r, h.head.x - 12.0f, h.head.y + 18.0f - hop, 9.0f);
        SDL_SetRenderDrawColor(r, 60, 40, 30, 255);
        c.label("$", h.head.x - 15.0f, h.head.y + 14.0f - hop, 1.5f);
        bubble(c, h.head.x + 6.0f, h.head.y - 12.0f, "shh", 1.5f);
        break; }
    case ActionId::Sleep: {
        const float x = targetX - 4.0f;
        SDL_SetRenderDrawColor(r, 235, 235, 245, 255);                      // pillow
        fillRect(r, x - 18.0f, gy - 10.0f, 16.0f, 8.0f);
        setColor(r, kSkin);
        fillCircle(r, x - 8.0f, gy - 12.0f, 6.5f);
        setColor(r, kHat);
        fillRect(r, x - 14.0f, gy - 19.0f, 12.0f, 5.0f);                    // the hat, on his face
        SDL_SetRenderDrawColor(r, 80, 110, 170, 255);                       // blanket
        fillRect(r, x - 2.0f, gy - 12.0f, 50.0f, 10.0f);
        SDL_SetRenderDrawColor(r, 120, 150, 205, 255);
        for (float k = 6.0f; k < 46.0f; k += 10.0f) fillRect(r, x + k, gy - 12.0f, 4.0f, 10.0f);
        for (int k = 0; k < 3; ++k) {                                       // Zzz
            const float ph = fract(t * 0.5f + static_cast<float>(k) / 3.0f);
            SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
            c.label("Z", x + 6.0f + ph * 22.0f, gy - 30.0f - ph * 40.0f, 1.5f + ph * 1.5f);
        }
        if (walking) { drawPerson(c, pa); }
        break; }
    case ActionId::Give: {
        pa.armFront = kPi * 0.5f;
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        const float ph = fract(t * 0.6f);
        const float gx = h.front.x + ph * 62.0f, gyy = h.front.y - 4.0f - std::sin(ph * kPi) * 18.0f;
        SDL_SetRenderDrawColor(r, 200, 50, 60, 255);
        fillRect(r, gx - 7.0f, gyy - 6.0f, 14.0f, 12.0f);
        SDL_SetRenderDrawColor(r, 250, 215, 80, 255);
        fillRect(r, gx - 1.0f, gyy - 6.0f, 2.0f, 12.0f);
        fillRect(r, gx - 7.0f, gyy - 1.0f, 14.0f, 2.0f);
        for (int k = 0; k < 3; ++k) {
            const float hp = fract(t * 0.7f + static_cast<float>(k) / 3.0f);
            heart(r, h.front.x + 62.0f + std::sin(hp * 7.0f + static_cast<float>(k)) * 8.0f, gy - 44.0f - hp * 30.0f, 3.0f + 2.0f * (1.0f - hp));
        }
        break; }
    case ActionId::Cross: {
        pa.legPhase = seconds * 8.0f; pa.legSwing = 0.6f; pa.lean = 3.0f; pa.armFront = kPi * 0.62f; pa.armBack = kPi * 0.62f;
        pa.x = lerp(homeX, c.zoneX(0.72f), clamp01(seconds / 3.5f));
        drawPerson(c, pa);
        for (float x = pa.x - 30.0f; x < pa.x + 32.0f; x += 2.0f) {         // the water, up to his waist
            const float wave = std::sin(x * 0.2f + seconds * 7.0f) * 2.0f;
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 60, 120, 190, 215);
            fillRect(r, x, gy - 17.0f + wave, 2.0f, 24.0f - wave);
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(r, 190, 225, 250, 255);
            fillRect(r, x, gy - 17.0f + wave, 2.0f, 2.0f);
        }
        for (int k = 0; k < 4; ++k) {
            const float ph = fract(seconds * 0.9f + static_cast<float>(k) * 0.25f);
            SDL_SetRenderDrawColor(r, 220, 240, 255, 255);
            strokeCircle(r, pa.x - 12.0f + static_cast<float>(k) * 9.0f, gy - 20.0f - ph * 22.0f, 2.0f + ph);
        }
        break; }
    case ActionId::Inspect: {
        pa.lean = 4.0f; pa.armBack = kPi * 0.4f;
        const float sweep = std::sin(t * 2.2f);
        pa.armFront = kPi * (0.55f + 0.06f * sweep);
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        const float mx = h.front.x + 16.0f + 14.0f * sweep, my = h.front.y + 2.0f;
        SDL_SetRenderDrawColor(r, 80, 60, 40, 255);
        thickLine(r, h.front.x, h.front.y, mx - 6.0f, my + 5.0f, 2.0f);
        SDL_SetRenderDrawColor(r, 150, 200, 230, 255);
        fillCircle(r, mx, my, 9.0f);
        SDL_SetRenderDrawColor(r, 60, 40, 30, 255);
        strokeCircle(r, mx, my, 9.0f); strokeCircle(r, mx, my, 8.0f);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        fillRect(r, mx - 4.0f, my - 5.0f, 3.0f, 3.0f);
        setColor(r, kCream);                                                // a clipboard in the other hand
        fillRect(r, h.back.x - 6.0f, h.back.y - 9.0f, 12.0f, 16.0f);
        SDL_SetRenderDrawColor(r, 60, 40, 30, 255);
        for (float ly = -5.0f; ly < 6.0f; ly += 4.0f) draw::line(r, h.back.x - 4.0f, h.back.y + ly, h.back.x + 4.0f, h.back.y + ly);
        break; }
    case ActionId::Shoot: {
        const float cycle = fract(t * 0.55f);
        const bool fired = cycle > 0.55f && cycle < 0.7f;
        pa.armFront = kPi * 0.5f; pa.armBack = kPi * 0.46f; pa.x = targetX - (fired ? 3.0f : 0.0f);
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        setColor(r, kOutline);                                              // rifle
        thickLine(r, h.back.x - 6.0f, h.back.y + 3.0f, h.front.x + 30.0f, h.front.y - 2.0f, 3.0f);
        setColor(r, kWood);
        thickLine(r, h.back.x - 12.0f, h.back.y + 6.0f, h.back.x - 2.0f, h.back.y + 2.0f, 4.0f);
        const float mx = h.front.x + 31.0f, my = h.front.y - 2.0f;
        if (fired) {
            SDL_SetRenderDrawColor(r, 255, 235, 100, 255);
            starBurst(r, mx + 4.0f, my, 2.0f, 12.0f, 8, t * 5.0f);
            SDL_SetRenderDrawColor(r, 255, 140, 40, 255);
            c.label("BANG", mx - 6.0f, my - 30.0f, 2.0f);
        }
        if (cycle > 0.6f) {                                                 // the bullet, and what it misses
            const float bp = clamp01((cycle - 0.6f) / 0.25f);
            SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
            const float bx = mx + 6.0f + bp * 140.0f;
            draw::line(r, bx, my, bx + 12.0f, my);
        }
        break; }
    case ActionId::Join: {
        pa.armFront = kPi * (0.78f + 0.12f * std::sin(t * 6.0f)); pa.armBack = kPi * (0.78f - 0.12f * std::sin(t * 6.0f));
        pa.headDx = std::sin(t * 3.0f);
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        for (int k = 0; k < 3; ++k) {
            const float hp = fract(t * 0.6f + static_cast<float>(k) / 3.0f);
            heart(r, h.head.x + 14.0f + static_cast<float>(k) * 12.0f, h.head.y - 6.0f - hp * 30.0f, 2.5f + 1.5f * (1.0f - hp));
        }
        break; }
    case ActionId::Wait: {
        pa.legSwing = 0.0f;
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        const float cx = h.head.x + 4.0f, cy = h.head.y - 34.0f;            // a clock, with a very slow second hand
        SDL_SetRenderDrawColor(r, 250, 248, 235, 255);
        fillCircle(r, cx, cy, 13.0f);
        SDL_SetRenderDrawColor(r, 60, 40, 30, 255);
        strokeCircle(r, cx, cy, 13.0f); strokeCircle(r, cx, cy, 12.0f);
        const float a = t * 2.4f;
        thickLine(r, cx, cy, cx + std::sin(a) * 10.0f, cy - std::cos(a) * 10.0f, 2.0f);
        thickLine(r, cx, cy, cx + std::sin(a / 12.0f) * 6.0f, cy - std::cos(a / 12.0f) * 6.0f, 2.0f);
        const int dots = 1 + static_cast<int>(t * 1.5f) % 3;
        for (int k = 0; k < dots; ++k) fillCircle(r, h.head.x + 14.0f + static_cast<float>(k) * 7.0f, h.head.y - 2.0f, 1.8f);
        break; }
    case ActionId::Build: {
        const float swing = std::sin(t * 8.0f);
        pa.armFront = kPi * (0.55f + 0.5f * (swing * 0.5f + 0.5f)); pa.armBack = kPi * 0.3f; pa.lean = 3.0f;
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        SDL_SetRenderDrawColor(r, 150, 100, 60, 255);                       // a plank being nailed down
        fillRect(r, pa.x + 14.0f, gy - 7.0f, 60.0f, 7.0f);
        SDL_SetRenderDrawColor(r, 90, 90, 100, 255);
        fillRect(r, pa.x + 32.0f, gy - 9.0f, 2.0f, 4.0f);
        setColor(r, kWood);                                                 // hammer
        thickLine(r, h.front.x, h.front.y, h.front.x + 9.0f, h.front.y + (swing > 0 ? -9.0f : 9.0f), 3.0f);
        SDL_SetRenderDrawColor(r, 120, 120, 130, 255);
        fillRect(r, h.front.x + 5.0f, h.front.y + (swing > 0 ? -14.0f : 7.0f), 9.0f, 6.0f);
        if (swing < -0.85f) {
            SDL_SetRenderDrawColor(r, 255, 235, 120, 255);
            starBurst(r, pa.x + 33.0f, gy - 8.0f, 2.0f, 9.0f, 7, t * 9.0f);
        }
        break; }
    case ActionId::Burn: {
        pa.armFront = kPi * 0.5f; pa.lean = 2.0f;
        const Hands h = drawPerson(c, pa);
        if (walking) break;
        setColor(r, kWood);                                                 // a torch
        thickLine(r, h.front.x, h.front.y, h.front.x + 5.0f, h.front.y - 16.0f, 3.0f);
        flame(r, h.front.x + 5.0f, h.front.y - 16.0f, 8.0f, t, 0.5f);
        const int fires = std::min(5, 1 + static_cast<int>(t * 1.2f));
        for (int k = 0; k < fires; ++k) flame(r, pa.x + 30.0f + static_cast<float>(k) * 14.0f, gy, 7.0f + static_cast<float>(k % 2) * 2.0f, t, static_cast<float>(k) * 1.9f);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        for (int k = 0; k < 5; ++k) {
            const float ph = fract(t * 0.5f + static_cast<float>(k) * 0.2f);
            SDL_SetRenderDrawColor(r, 90, 90, 95, static_cast<Uint8>(150 * (1.0f - ph)));
            fillCircle(r, pa.x + 50.0f + std::sin(ph * 8.0f + static_cast<float>(k)) * 6.0f, gy - 14.0f - ph * 60.0f, 4.0f + ph * 8.0f);
        }
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        break; }
    case ActionId::None:
    case ActionId::Count:
        break;
    }
}

void drawOutcomeBadge(const StageContext& c, int outcome, float seconds) {
    if (outcome == 0 || seconds < 0.6f) return;
    SDL_Renderer* r = c.r;
    const float cx = c.area.x + c.area.w - 40.0f, cy = c.area.y + 38.0f;
    const float grow = clamp01((seconds - 0.6f) / 0.3f);
    if (outcome > 0) {
        SDL_SetRenderDrawColor(r, 255, 235, 110, 255);
        starBurst(r, cx, cy, 14.0f * grow, 26.0f * grow, 12, seconds * 0.8f);
        SDL_SetRenderDrawColor(r, 90, 170, 80, 255);
        fillCircle(r, cx, cy, 13.0f * grow);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        fillRect(r, cx - 7.0f * grow, cy - 1.5f * grow, 14.0f * grow, 3.0f * grow);
        fillRect(r, cx - 1.5f * grow, cy - 7.0f * grow, 3.0f * grow, 14.0f * grow);
    } else {
        SDL_SetRenderDrawColor(r, 85, 90, 105, 255);
        fillCircle(r, cx - 10.0f * grow, cy, 11.0f * grow);
        fillCircle(r, cx + 3.0f * grow, cy - 6.0f * grow, 14.0f * grow);
        fillCircle(r, cx + 15.0f * grow, cy + 1.0f * grow, 10.0f * grow);
        fillRect(r, cx - 20.0f * grow, cy, 44.0f * grow, 8.0f * grow);
        SDL_SetRenderDrawColor(r, 120, 170, 235, 255);
        for (int k = 0; k < 4; ++k) {
            const float ph = fract(seconds * 1.4f + static_cast<float>(k) * 0.25f);
            fillRect(r, cx - 14.0f + static_cast<float>(k) * 10.0f, cy + 10.0f + ph * 22.0f, 2.0f, 5.0f);
        }
    }
}

}  // namespace stage
