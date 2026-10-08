#include "Game.h"
#include <algorithm>
#include <iostream>

namespace {
    // Bottom panel layout (pixels)
    constexpr float       kPadding        = 24.0f;
    constexpr float       kButtonHeight   = 52.0f;
    constexpr float       kButtonGap      = 12.0f;
    constexpr float       kTextGap        = 12.0f;  // space between narrative text and buttons
    constexpr float       kNarrativeScale = 2.0f;   // 8px font drawn at 16px
    constexpr std::size_t kMaxChoices     = 4;      // matches keys 1-4 and the 2x2 button grid
}

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

    loadSampleEvents();
    showEvent(0);

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
        switch (event.type) {
        case SDL_EVENT_QUIT:
            m_running = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.repeat) break;  // ignore held-down key repeats
            if (event.key.key == SDLK_ESCAPE) {
                m_running = false;
            } else if (event.key.key >= SDLK_1 && event.key.key <= SDLK_4) {
                selectChoice(static_cast<std::size_t>(event.key.key - SDLK_1));
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            handleMouseMotion(event.motion.x, event.motion.y);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                handleMouseClick(event.button.x, event.button.y);
            }
            break;

        default:
            break;
        }
    }
}

void Game::update() {
    // TODO: Trail::advance(), Party state updates, etc. go here later.
}

// ---------------------------------------------------------------------------
// Bottom panel: events, buttons, input
// ---------------------------------------------------------------------------

void Game::loadSampleEvents() {
    // PLACEHOLDER content so the UI has something to show. Drafted with AI
    // assistance (logged in docs/ai-dev-log.md). Week 3 replaces this with
    // events loaded from files in data/.
    m_events.emplace_back(
        "You have reached the Pecos River. The river is wide, brown, and has not "
        "indicated whether it intends to be a problem.",
        std::vector<Choice>{
            Choice("Ford the river"),
            Choice("Caulk the wagon and float it"),
            Choice("Wait for conditions to improve"),
            Choice("Ask the river what it wants")
        });

    m_events.emplace_back(
        "One of your oxen has stopped walking. It is looking at you the way a man "
        "looks at a bad investment.",
        std::vector<Choice>{
            Choice("Give the ox a motivational speech"),
            Choice("Redistribute its load"),
            Choice("Leave it a note and move on")
        });

    m_events.emplace_back(
        "A stranger at the edge of camp offers to repair your wagon wheel for free. "
        "He has too many teeth.",
        std::vector<Choice>{
            Choice("Accept his offer"),
            Choice("Repair it yourself")
        });
}

void Game::showEvent(std::size_t index) {
    if (m_events.empty() || index >= m_events.size()) return;

    m_currentEvent = index;
    m_buttons.clear();

    // Rects are placeholders here; layoutButtons() positions them.
    const std::vector<Choice>& choices = m_events[index].getChoices();
    for (std::size_t i = 0; i < choices.size() && i < kMaxChoices; ++i) {
        m_buttons.emplace_back(choices[i].getLabel(), SDL_FRect{ 0.0f, 0.0f, 0.0f, 0.0f });
    }
    layoutButtons();

    // Refresh hover state so a button appearing under the cursor lights up immediately.
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    SDL_GetMouseState(&mouseX, &mouseY);
    handleMouseMotion(mouseX, mouseY);
}

void Game::layoutButtons() {
    // Buttons sit in a 2-column grid anchored to the bottom of the window,
    // so the narrative text above gets whatever room is left.
    const float width = (static_cast<float>(m_windowWidth) - 2.0f * kPadding - kButtonGap) / 2.0f;
    const int   count = static_cast<int>(m_buttons.size());
    const int   rows  = (count + 1) / 2;

    const float gridBottom = static_cast<float>(m_windowHeight) - kPadding;
    const float gridHeight = rows > 0 ? rows * kButtonHeight + (rows - 1) * kButtonGap : 0.0f;
    m_buttonsTop = gridBottom - gridHeight;

    for (int i = 0; i < count; ++i) {
        const int col = i % 2;
        const int row = i / 2;

        SDL_FRect rect;
        rect.x = kPadding + col * (width + kButtonGap);
        rect.y = m_buttonsTop + row * (kButtonHeight + kButtonGap);
        rect.w = width;
        rect.h = kButtonHeight;
        m_buttons[static_cast<std::size_t>(i)].setRect(rect);
    }
}

void Game::selectChoice(std::size_t index) {
    if (index >= m_buttons.size()) return;  // key pressed for a choice this event doesn't have

    std::cout << "Chose [" << (index + 1) << "]: " << m_buttons[index].getLabel() << "\n";

    // TODO (Week 3): apply the Choice's effects to the Party, then ask the Trail
    // for the next Event. For now just cycle through the sample events.
    showEvent((m_currentEvent + 1) % m_events.size());
}

void Game::handleMouseMotion(float x, float y) {
    for (Button& button : m_buttons) {
        button.setHovered(button.contains(x, y));
    }
}

void Game::handleMouseClick(float x, float y) {
    for (std::size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].contains(x, y)) {
            selectChoice(i);
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

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
    const float topHeight = static_cast<float>(m_windowHeight) * kTopPanelRatio;

    SDL_FRect bottomRect;
    bottomRect.x = 0;
    bottomRect.y = topHeight;
    bottomRect.w = static_cast<float>(m_windowWidth);
    bottomRect.h = static_cast<float>(m_windowHeight) - topHeight;

    SDL_SetRenderDrawColor(m_renderer, 245, 240, 225, 255); // cream/journal page
    SDL_RenderFillRect(m_renderer, &bottomRect);

    // Thin divider between the two panels
    SDL_FRect divider{ 0.0f, topHeight, static_cast<float>(m_windowWidth), 3.0f };
    SDL_SetRenderDrawColor(m_renderer, 120, 100, 70, 255);
    SDL_RenderFillRect(m_renderer, &divider);

    // Narrative text, limited to the space above the buttons
    if (m_currentEvent < m_events.size()) {
        const float textTop   = topHeight + kPadding;
        const float available = m_buttonsTop - kTextGap - textTop;
        const int   maxLines  = std::max(1, static_cast<int>(available / m_text.lineHeight(kNarrativeScale)));

        SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
        m_text.drawWrapped(m_renderer, m_events[m_currentEvent].getText(),
                           kPadding, textTop,
                           static_cast<float>(m_windowWidth) - 2.0f * kPadding,
                           kNarrativeScale, maxLines);
    }

    for (std::size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].draw(m_renderer, m_text, static_cast<int>(i) + 1);
    }
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
