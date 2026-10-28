#include <vector>
#include "SceneRenderer.h"

#include "DrawUtil.h"
#include "SceneActors.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace {
    using namespace draw;

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
            draw::line(r, cx - std::cos(a) * radius, cy - std::sin(a) * radius,
                              cx + std::cos(a) * radius, cy + std::sin(a) * radius);
        }
        fillCircle(r, cx, cy, 2.5f);
    }

}

namespace {
    struct Layout {
        float horizon, roadY, groundY;
    };

    Layout layoutFor(const SDL_FRect& area) {
        Layout l;
        l.horizon = area.y + area.h * 0.62f;
        l.roadY = l.horizon + (area.y + area.h - l.horizon) * 0.45f;
        l.groundY = l.roadY + (area.y + area.h - l.roadY) * 0.40f;
        return l;
    }

    // Sky, stars, sun or moon, hills, ground and road.
    void drawBackdrop(SDL_Renderer* r, const SDL_FRect& area, TimePart part, const Layout& l) {
        const int index = static_cast<int>(part);
        const Palette& palette = kPalettes[index];

        setColor(r, palette.sky);
        fillRect(r, area.x, area.y, area.w, area.h);

        if (part == TimePart::Night) {   // stars
            SDL_SetRenderDrawColor(r, 235, 235, 255, 255);
            static const float stars[][2] = {
                {0.06f,0.10f},{0.15f,0.30f},{0.24f,0.08f},{0.33f,0.22f},{0.41f,0.12f},{0.50f,0.30f},
                {0.58f,0.07f},{0.66f,0.24f},{0.83f,0.09f},{0.91f,0.27f},{0.96f,0.12f},{0.45f,0.38f} };
            for (const auto& s : stars) fillRect(r, area.x + s[0] * area.w, area.y + s[1] * area.h, 2.0f, 2.0f);
        }

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

        setColor(r, palette.hills);
        fillDome(r, area.x + area.w * 0.22f, l.horizon, 120.0f, 46.0f);
        fillDome(r, area.x + area.w * 0.70f, l.horizon, 150.0f, 58.0f);
        setColor(r, palette.ground);
        fillRect(r, area.x, l.horizon, area.w, area.y + area.h - l.horizon);
        setColor(r, palette.road);
        fillRect(r, area.x, l.roadY, area.w, (area.y + area.h - l.roadY) * 0.55f);
    }

    // The covered wagon. 'rolling' makes it bob and its wheels turn; parked wagons sit still.
    void drawWagon(SDL_Renderer* r, float wagonX, const Layout& l, float seconds, bool rolling) {
        const float bob = rolling ? std::sin(seconds * 6.0f) * 1.5f : 0.0f;
        const float angle = rolling ? seconds * 3.0f : 0.0f;
        const float wagonBottom = l.groundY - 14.0f + bob;
        setColor(r, kWagonWood);
        fillRect(r, wagonX, wagonBottom - 34.0f, 96.0f, 30.0f);                  // body
        setColor(r, kCanvas);
        fillDome(r, wagonX + 48.0f, wagonBottom - 34.0f, 48.0f, 36.0f);          // canvas cover
        setColor(r, kOutline);
        fillRect(r, wagonX + 96.0f, wagonBottom - 24.0f, 22.0f, 3.0f);           // hitch pole
        drawWheel(r, wagonX + 22.0f, l.groundY - 2.0f + bob, 16.0f, angle);
        drawWheel(r, wagonX + 76.0f, l.groundY - 2.0f + bob, 16.0f, angle);
    }

    // Dread tints everything red as sanity drops.
    void drawDread(SDL_Renderer* r, const SDL_FRect& area, float dread) {
        if (dread <= 0.0f) return;
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 130, 0, 25, static_cast<Uint8>(std::min(1.0f, dread) * 85.0f));
        fillRect(r, area.x, area.y, area.w, area.h);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }
}

void SceneRenderer::releaseResources() {
    if (m_pixelTexture) SDL_DestroyTexture(m_pixelTexture);
    m_pixelTexture = nullptr;
    m_pixelW = m_pixelH = 0;
}

bool SceneRenderer::beginPixelPass(SDL_Renderer* r) const {
    int outW = 0, outH = 0;
    if (!SDL_GetRenderOutputSize(r, &outW, &outH) || outW <= 0 || outH <= 0) return false;
    const int w = (outW + kPixelSize - 1) / kPixelSize;
    const int h = (outH + kPixelSize - 1) / kPixelSize;
    if (!m_pixelTexture || w != m_pixelW || h != m_pixelH) {
        if (m_pixelTexture) SDL_DestroyTexture(m_pixelTexture);
        m_pixelTexture = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
        m_pixelW = w; m_pixelH = h;
        if (!m_pixelTexture) return false;   // fall back to smooth drawing
        SDL_SetTextureScaleMode(m_pixelTexture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(m_pixelTexture, SDL_BLENDMODE_BLEND);
    }
    SDL_GetRenderScale(r, &m_savedScaleX, &m_savedScaleY);   // (each render target keeps its own scale)
    if (!SDL_SetRenderTarget(r, m_pixelTexture)) return false;
    SDL_SetRenderScale(r, 1.0f / kPixelSize, 1.0f / kPixelSize);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    m_inPass = true;
    draw::pixelStep() = static_cast<float>(kPixelSize);
    return true;
}

void SceneRenderer::endPixelPass(SDL_Renderer* r, const SDL_FRect& area) const {
    if (!m_inPass) return;
    m_inPass = false;
    draw::pixelStep() = 1.0f;
    SDL_SetRenderClipRect(r, nullptr);
    SDL_SetRenderTarget(r, nullptr);
    SDL_SetRenderScale(r, m_savedScaleX, m_savedScaleY);
    const float k = static_cast<float>(kPixelSize);
    const SDL_FRect src{ area.x / k, area.y / k, area.w / k, area.h / k };
    SDL_RenderTexture(r, m_pixelTexture, &src, &area);
}

void SceneRenderer::drawScene(SDL_Renderer* r, const SDL_FRect& area, TimePart part,
                              Uint64 timeMs, float dread, float travel) const {
    const Layout l = layoutFor(area);
    const float seconds = static_cast<float>(timeMs) / 1000.0f;
    beginPixelPass(r);

    // Keep everything (the hills especially) inside the frame.
    const SDL_Rect clip{ static_cast<int>(area.x), static_cast<int>(area.y),
                         static_cast<int>(area.w), static_cast<int>(area.h) };
    SDL_SetRenderClipRect(r, &clip);

    drawBackdrop(r, area, part, l);

    // The wagon bobs gently and its wheels roll. On the title screen it drives across and off the edge.
    const float wagonX = travel >= 0.0f
        ? area.x - 290.0f + (area.w + 290.0f) * std::min(1.0f, travel)
        : area.x + area.w * 0.12f;
    drawWagon(r, wagonX, l, seconds, true);

    // Two oxen, watching
    StageContext c;
    c.r = r;
    const float stride = seconds * 6.0f;
    stage::drawOx(c, wagonX + 112.0f, l.groundY + 2.0f, stride);
    stage::drawOx(c, wagonX + 196.0f, l.groundY + 2.0f, stride + kPi);

    drawDread(r, area, dread);

    SDL_SetRenderClipRect(r, nullptr);
    endPixelPass(r, area);
    setColor(r, kOutline);
    SDL_RenderRect(r, &area);
}

void SceneRenderer::drawStage(SDL_Renderer* r, const TextRenderer& text, const SDL_FRect& area, TimePart part,
                              Uint64 timeMs, float dread, const Stage& stage) const {
    const Layout l = layoutFor(area);
    const float seconds = static_cast<float>(timeMs) / 1000.0f;
    beginPixelPass(r);

    const SDL_Rect clip{ static_cast<int>(area.x), static_cast<int>(area.y),
                         static_cast<int>(area.w), static_cast<int>(area.h) };
    SDL_SetRenderClipRect(r, &clip);

    drawBackdrop(r, area, part, l);

    StageContext c;
    c.r = r;
    c.text = &text;
    c.area = area;
    c.t = seconds;
    c.horizon = l.horizon;
    c.roadY = l.roadY;
    c.groundY = l.groundY;
    c.wagonX = area.x + area.w * 0.04f;
    c.part = part;
    std::vector<DeferredText> labels;
    if (m_inPass) c.deferred = &labels;

    // The wagon waits at the left while whatever is happening happens on the right.
    drawWagon(r, c.wagonX, l, seconds, false);

    if (stage.scene == SceneId::Trail) {   // no particular scene: the oxen plod about
        stage::drawOx(c, c.zoneX(0.55f), l.groundY + 2.0f, seconds * 6.0f);
        stage::drawOx(c, c.zoneX(0.55f) + 88.0f, l.groundY + 2.0f, seconds * 6.0f + kPi);
    } else {
        stage::drawSceneActor(c, stage.scene);
    }
    stage::drawAction(c, stage.action, stage.actionSeconds);
    if (stage.action != ActionId::None) stage::drawOutcomeBadge(c, stage.outcome, stage.actionSeconds);

    drawDread(r, area, dread);

    SDL_SetRenderClipRect(r, nullptr);
    endPixelPass(r, area);
    if (!labels.empty()) {
        const SDL_Rect textClip{ static_cast<int>(area.x), static_cast<int>(area.y),
                                 static_cast<int>(area.w), static_cast<int>(area.h) };
        SDL_SetRenderClipRect(r, &textClip);
        for (const DeferredText& d : labels) {
            SDL_SetRenderDrawColor(r, d.color.r, d.color.g, d.color.b, d.color.a);
            text.drawLine(r, d.text, d.x, d.y, d.scale);
        }
        SDL_SetRenderClipRect(r, nullptr);
    }
    setColor(r, kOutline);
    SDL_RenderRect(r, &area);
}

void SceneRenderer::drawPortrait(SDL_Renderer* r, const SDL_FRect& area, int memberIndex,
                                 PartyMember::Status status) const {
    const bool dead = status == PartyMember::Status::Dead;
    const bool ill  = status == PartyMember::Status::Ill;

    // Card
    if (dead)     SDL_SetRenderDrawColor(r, 190, 190, 190, 255);
    else if (ill) SDL_SetRenderDrawColor(r, 205, 218, 170, 255);
    else          SDL_SetRenderDrawColor(r, 236, 224, 196, 255);
    fillRect(r, area.x, area.y, area.w, area.h);

    const float cx = area.x + area.w * 0.5f;
    const float cy = area.y + area.h * 0.60f;
    const float radius = memberIndex == 2 ? 14.0f : 17.0f;  // Junior is smaller

    // Face
    if (dead)     SDL_SetRenderDrawColor(r, 205, 205, 205, 255);
    else if (ill) SDL_SetRenderDrawColor(r, 168, 200, 140, 255);
    else          SDL_SetRenderDrawColor(r, 238, 200, 162, 255);
    fillCircle(r, cx, cy, radius);

    // What makes each member recognisable
    switch (memberIndex) {
    case 0:  // Ma: blue bonnet with a bow
        SDL_SetRenderDrawColor(r, 120, 140, 175, 255);
        fillDome(r, cx, cy - 3.0f, radius + 3.0f, 19.0f);
        SDL_SetRenderDrawColor(r, 200, 90, 90, 255);
        fillRect(r, cx - 4.0f, cy - radius - 3.0f, 8.0f, 4.0f);
        break;
    case 1:  // Pa: hat and moustache
        SDL_SetRenderDrawColor(r, 70, 50, 38, 255);
        fillRect(r, cx - 11.0f, cy - radius - 9.0f, 22.0f, 12.0f);   // crown
        fillRect(r, cx - 19.0f, cy - radius + 2.0f, 38.0f, 4.0f);    // brim
        SDL_SetRenderDrawColor(r, 90, 60, 40, 255);
        fillRect(r, cx - 8.0f, cy + 5.0f, 16.0f, 3.0f);              // moustache
        break;
    case 2:  // Junior: messy hair
        SDL_SetRenderDrawColor(r, 120, 72, 40, 255);
        fillDome(r, cx, cy - 4.0f, radius, 14.0f);
        draw::line(r, cx - 9.0f, cy - radius + 2.0f, cx - 13.0f, cy - radius - 5.0f);
        draw::line(r, cx,        cy - radius,        cx + 1.0f,  cy - radius - 7.0f);
        draw::line(r, cx + 9.0f, cy - radius + 2.0f, cx + 13.0f, cy - radius - 5.0f);
        break;
    default:  // Granny: white hair, bun and glasses
        SDL_SetRenderDrawColor(r, 238, 238, 238, 255);
        fillDome(r, cx, cy - 4.0f, radius + 1.0f, 15.0f);
        fillCircle(r, cx, cy - radius - 5.0f, 6.0f);
        break;
    }

    // Eyes and mouth
    SDL_SetRenderDrawColor(r, 50, 35, 25, 255);
    if (dead) {
        for (int side = -1; side <= 1; side += 2) {              // crossed-out eyes
            const float ex = cx + static_cast<float>(side) * 6.0f;
            draw::line(r, ex - 3.0f, cy - 5.0f, ex + 3.0f, cy + 1.0f);
            draw::line(r, ex - 3.0f, cy + 1.0f, ex + 3.0f, cy - 5.0f);
        }
        fillRect(r, cx - 5.0f, cy + 8.0f, 10.0f, 2.0f);          // flat mouth
    } else {
        fillRect(r, cx - 8.0f, cy - 3.0f, 3.0f, ill ? 2.0f : 4.0f);   // eyes (droopy when ill)
        fillRect(r, cx + 5.0f, cy - 3.0f, 3.0f, ill ? 2.0f : 4.0f);
        if (ill) {
            fillRect(r, cx - 5.0f, cy + 8.0f, 4.0f, 2.0f);       // wobbly mouth
            fillRect(r, cx - 1.0f, cy + 9.0f, 4.0f, 2.0f);
            fillRect(r, cx + 3.0f, cy + 8.0f, 3.0f, 2.0f);
        } else {
            fillRect(r, cx - 5.0f, cy + 8.0f, 10.0f, 2.0f);      // small smile
            fillRect(r, cx - 6.0f, cy + 7.0f, 2.0f, 2.0f);
            fillRect(r, cx + 4.0f, cy + 7.0f, 2.0f, 2.0f);
        }
    }
    if (memberIndex == 3 && !dead) {                             // Granny's glasses
        strokeCircle(r, cx - 6.5f, cy - 1.5f, 5.0f);
        strokeCircle(r, cx + 6.5f, cy - 1.5f, 5.0f);
    }

    if (ill) {                                                   // sweat drop
        SDL_SetRenderDrawColor(r, 110, 170, 230, 255);
        fillCircle(r, cx + radius - 1.0f, cy - 9.0f, 3.0f);
    }
    if (dead) {                                                  // halo
        SDL_SetRenderDrawColor(r, 235, 190, 60, 255);
        strokeEllipse(r, cx, area.y + 7.0f, 11.0f, 3.5f);
        strokeEllipse(r, cx, area.y + 7.0f, 11.0f, 2.5f);
    }

    SDL_SetRenderDrawColor(r, 50, 35, 25, 255);
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
