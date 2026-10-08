#pragma once
#include <SDL3/SDL.h>
#include <cstddef>
#include <vector>

#include "Button.h"
#include "Event.h"
#include "TextRenderer.h"

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

    // Bottom panel: narrative text and numbered choice buttons
    void loadSampleEvents();
    void showEvent(std::size_t index);
    void layoutButtons();
    void selectChoice(std::size_t index);
    void handleMouseMotion(float x, float y);
    void handleMouseClick(float x, float y);

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    bool m_running = false;

    int m_windowWidth  = 0;
    int m_windowHeight = 0;

    TextRenderer        m_text;
    std::vector<Event>  m_events;
    std::vector<Button> m_buttons;
    std::size_t         m_currentEvent = 0;
    float               m_buttonsTop   = 0.0f;  // y of the first button row, set by layoutButtons()

    // Split ratio: top panel takes this fraction of the window height.
    // Matches the proposal's ~60% top / ~40% bottom layout.
    static constexpr float kTopPanelRatio = 0.6f;
};
