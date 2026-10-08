#pragma once
#include <SDL3/SDL.h>

class Game {
public:
    Game();
    ~Game();

    bool init(const char* title, int width, int height);
    void run();

private:
    void handleInput();
    void update();
    void renderTop();
    void renderBottom();
    void render();
    void shutdown();

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    bool m_running = false;

    int m_windowWidth  = 0;
    int m_windowHeight = 0;

    // Split ratio: top panel takes this fraction of the window height.
    // Matches the proposal's ~60% top / ~40% bottom layout.
    static constexpr float kTopPanelRatio = 0.6f;
};
