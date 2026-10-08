#include "Game.h"
#include <iostream>

Game::Game() = default;

Game::~Game() {
    shutdown();
}

bool Game::init(const char* title, int width, int height) {
    // SDL3 change from SDL2: SDL_Init returns bool (true = success),
    // not an int where 0 = success. Easy mistake if you're following
    // older SDL2 tutorials.
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    m_windowWidth  = width;
    m_windowHeight = height;

    // SDL3 change: SDL_CreateWindow no longer takes x/y position
    // arguments (SDL2 had them before flags). Position is set
    // separately if you need it, or just let the OS place it.
    m_window = SDL_CreateWindow(title, width, height, 0);
    if (!m_window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    // SDL3 change: SDL_CreateRenderer takes (window, driver_name)
    // where driver_name is usually just nullptr to let SDL pick.
    // SDL2 had an extra integer flags parameter here.
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    m_running = true;
    return true;
}

void Game::run() {
    while (m_running) {
        handleInput();
        update();
        render();
    }
}

void Game::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // SDL3 change: event type constants are now SDL_EVENT_* instead
        // of SDL_* (e.g. SDL_EVENT_QUIT instead of SDL_QUIT).
        if (event.type == SDL_EVENT_QUIT) {
            m_running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                m_running = false;
            }
            // TODO: route number keys (1-4) to choice selection
            // once the Event/Choice system exists.
        }
    }
}

void Game::update() {
    // TODO: Trail::advance(), Party state updates, etc. go here later.
}

void Game::renderTop() {
    // Placeholder top panel: a plain rect so you can see the split
    // before any real wagon/trail art exists.
    SDL_FRect topRect;
    topRect.x = 0;
    topRect.y = 0;
    topRect.w = static_cast<float>(m_windowWidth);
    topRect.h = static_cast<float>(m_windowHeight) * kTopPanelRatio;

    SDL_SetRenderDrawColor(m_renderer, 210, 190, 150, 255); // parchment tan
    SDL_RenderFillRect(m_renderer, &topRect);

    // TODO: draw wagon sprite, trail dots, party row, inventory box here.
}

void Game::renderBottom() {
    float topHeight = static_cast<float>(m_windowHeight) * kTopPanelRatio;

    SDL_FRect bottomRect;
    bottomRect.x = 0;
    bottomRect.y = topHeight;
    bottomRect.w = static_cast<float>(m_windowWidth);
    bottomRect.h = static_cast<float>(m_windowHeight) - topHeight;

    SDL_SetRenderDrawColor(m_renderer, 245, 240, 225, 255); // cream/journal page
    SDL_RenderFillRect(m_renderer, &bottomRect);

    // TODO: draw narrative text + choice buttons here. Text rendering
    // needs SDL_ttf (or similar) added as a dependency later.
}

void Game::render() {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    renderTop();
    renderBottom();

    SDL_RenderPresent(m_renderer);
}

void Game::shutdown() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}
