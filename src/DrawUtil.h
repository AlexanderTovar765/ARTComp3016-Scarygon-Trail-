#pragma once
#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

// Small drawing helpers shared by the scene artwork. Everything on screen is built from
// rectangles, circles and lines, so there are no image files.
namespace draw {
    constexpr float kPi = 3.14159265f;

    inline void setColor(SDL_Renderer* r, SDL_Color c, Uint8 alpha = 255) {
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, alpha);
    }

    // Size of one visible "pixel" in scene units. 1 for smooth drawing; the pixel-art pass sets it to
    // its pixel size so that rows and thin shapes never fall between two pixels and vanish.
    inline float& pixelStep() { static float step = 1.0f; return step; }

    inline void fillRect(SDL_Renderer* r, float x, float y, float w, float h) {
        const float st = pixelStep();
        if (w > 0.0f && w < st) w = st;
        if (h > 0.0f && h < st) h = st;
        const SDL_FRect rect{ x, y, w, h };
        SDL_RenderFillRect(r, &rect);
    }

    // A one-pixel line. Inside the pixel-art pass SDL's own thin lines can vanish, so build it from blocks.
    inline void line(SDL_Renderer* r, float x0, float y0, float x1, float y1) {
        const float st = pixelStep();
        if (st <= 1.0f) { SDL_RenderLine(r, x0, y0, x1, y1); return; }
        const float dx = x1 - x0, dy = y1 - y0;
        const int n = std::max(1, static_cast<int>(std::ceil(std::max(std::fabs(dx), std::fabs(dy)) / st)));
        for (int i = 0; i <= n; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(n);
            fillRect(r, x0 + dx * t - st * 0.5f, y0 + dy * t - st * 0.5f, st, st);
        }
    }

    inline void strokeRect(SDL_Renderer* r, const SDL_FRect& b) {
        const float st = std::max(1.0f, pixelStep());
        if (st <= 1.0f) { SDL_RenderRect(r, &b); return; }
        fillRect(r, b.x, b.y, b.w, st);
        fillRect(r, b.x, b.y + b.h - st, b.w, st);
        fillRect(r, b.x, b.y, st, b.h);
        fillRect(r, b.x + b.w - st, b.y, st, b.h);
    }

    inline void fillCircle(SDL_Renderer* r, float cx, float cy, float radius) {
        const float st = pixelStep();
        for (float dy = -radius; dy <= radius; dy += st) {
            const float halfWidth = std::sqrt(std::max(0.0f, radius * radius - dy * dy));
            fillRect(r, cx - halfWidth, cy + dy, halfWidth * 2.0f, st);
        }
    }

    inline void fillEllipse(SDL_Renderer* r, float cx, float cy, float rx, float ry) {
        if (ry <= 0.0f) return;
        const float st = pixelStep();
        for (float dy = -ry; dy <= ry; dy += st) {
            const float t = dy / ry;
            const float halfWidth = rx * std::sqrt(std::max(0.0f, 1.0f - t * t));
            fillRect(r, cx - halfWidth, cy + dy, halfWidth * 2.0f, st);
        }
    }

    inline void strokeEllipse(SDL_Renderer* r, float cx, float cy, float rx, float ry) {
        const int segments = 32;
        for (int i = 0; i < segments; ++i) {
            const float a0 = 2.0f * kPi * static_cast<float>(i) / segments;
            const float a1 = 2.0f * kPi * static_cast<float>(i + 1) / segments;
            line(r, cx + std::cos(a0) * rx, cy + std::sin(a0) * ry,
                              cx + std::cos(a1) * rx, cy + std::sin(a1) * ry);
        }
    }

    inline void strokeCircle(SDL_Renderer* r, float cx, float cy, float radius) {
        strokeEllipse(r, cx, cy, radius, radius);
    }

    // A filled half-ellipse sitting on baseY (hills, the wagon's canvas cover, bushes).
    inline void fillDome(SDL_Renderer* r, float cx, float baseY, float halfWidth, float height) {
        const float st = pixelStep();
        for (float row = 0.0f; row < height; row += st) {
            const float t = row / height;
            const float w = halfWidth * std::sqrt(std::max(0.0f, 1.0f - t * t));
            fillRect(r, cx - w, baseY - row, w * 2.0f, st);
        }
    }

    // A line with some thickness (SDL's own lines are one pixel wide).
    inline void thickLine(SDL_Renderer* r, float x0, float y0, float x1, float y1, float thickness) {
        const float dx = x1 - x0, dy = y1 - y0;
        const float length = std::sqrt(dx * dx + dy * dy);
        if (length < 0.001f || thickness <= 1.0f) { line(r, x0, y0, x1, y1); return; }
        const float st = pixelStep();
        if (st > 1.0f) {   // pixel-art pass: stamp square blocks along the line
            const float size = std::max(st, std::floor(thickness / st + 0.5f) * st);
            const int n = std::max(1, static_cast<int>(std::ceil(length / st)));
            for (int i = 0; i <= n; ++i) {
                const float t = static_cast<float>(i) / static_cast<float>(n);
                fillRect(r, x0 + dx * t - size * 0.5f, y0 + dy * t - size * 0.5f, size, size);
            }
            return;
        }
        const float nx = -dy / length, ny = dx / length;
        for (float k = -thickness * 0.5f; k <= thickness * 0.5f; k += pixelStep()) {
            line(r, x0 + nx * k, y0 + ny * k, x1 + nx * k, y1 + ny * k);
        }
    }

    // Spikes radiating from a point: impacts, sparkles, flashes.
    inline void starBurst(SDL_Renderer* r, float cx, float cy, float inner, float outer, int spikes, float rotation) {
        for (int i = 0; i < spikes; ++i) {
            const float a = rotation + 2.0f * kPi * static_cast<float>(i) / static_cast<float>(spikes);
            line(r, cx + std::cos(a) * inner, cy + std::sin(a) * inner,
                              cx + std::cos(a) * outer, cy + std::sin(a) * outer);
        }
    }

    inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
    inline float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
    inline float fract(float v) { return v - std::floor(v); }
}
