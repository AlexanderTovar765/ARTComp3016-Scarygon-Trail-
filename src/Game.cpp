#include "Game.h"
#include <algorithm>
#include <iostream>

namespace {
    const char* const kEventFile = "data/events.txt";  // relative to the working directory

    // Bottom panel layout (pixels)
    constexpr float       kPadding        = 24.0f;
    constexpr float       kButtonHeight   = 52.0f;
    constexpr float       kButtonGap      = 12.0f;
    constexpr float       kTextGap        = 12.0f;  // space between narrative text and buttons
    constexpr float       kNarrativeScale = 2.0f;   // 8px font drawn at 16px
    constexpr std::size_t kMaxChoices     = 4;      // matches keys 1-4 and the 2x2 button grid

    // Top panel (HUD) layout
    constexpr float kHeadingScale = 3.0f;
    constexpr float kStatScale    = 2.0f;
    constexpr float kStatBarX     = 140.0f;
    constexpr float kStatBarWidth = 240.0f;
    constexpr float kStatBarHeight = 16.0f;
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

    const bool loaded = m_trail.loadFromFile(kEventFile);
    for (const std::string& warning : m_trail.getWarnings()) {
        std::cerr << "[events] " << warning << "\n";
    }

    if (loaded) {
        startGame();
    } else {
        // Show the problem in the window instead of crashing or closing silently.
        showEnding(std::string("Could not load ") + kEventFile + ". Make sure a data folder "
                   "containing events.txt sits next to the game, then start it again. "
                   "Details are in the console window.", false);
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
    // Nothing runs on a timer yet: the game advances when the player picks a button.
}

// ---------------------------------------------------------------------------
// Game flow: Event -> Result -> (next Event | DaySummary) -> ... -> Ending
// ---------------------------------------------------------------------------

void Game::startGame() {
    m_party.reset();
    m_trail.reset();
    showCurrentEvent();
}

void Game::showCurrentEvent() {
    const Event* event = m_trail.currentEvent();

    if (!event) {
        // A gap in the data file. Don't crash: say which slot is empty and let play continue.
        m_phase = Phase::Result;
        showScreen("No event is written for Day " + std::to_string(m_trail.currentDay()) + ", " +
                   partName(m_trail.currentPart()) + ". Add one to " + kEventFile +
                   ". The day carries on without it.", { "Continue" });
        return;
    }

    m_phase = Phase::Event;
    std::vector<std::string> labels;
    for (const Choice& choice : event->getChoices()) {
        labels.push_back(choice.getLabel());
    }
    showScreen(event->getText(), labels);
}

void Game::onEventChoice(std::size_t index) {
    const Event* event = m_trail.currentEvent();
    if (!event || index >= event->getChoices().size()) return;

    const Choice& choice = event->getChoices()[index];
    const Effects effects = choice.getEffects();

    m_party.apply(effects);
    const Effects upkeep = m_party.advanceTime();  // the party eats (and may starve)

    std::cout << "Day " << m_trail.currentDay() << ", " << partName(m_trail.currentPart())
              << ": chose [" << (index + 1) << "] " << choice.getLabel()
              << " (" << effects.describe() << ")\n";

    std::string text = choice.getResultText() + " (" + effects.describe() + ") "
                     + "The party eats (" + upkeep.describe() + ").";
    if (upkeep.health < 0) {
        text += " The party is starving.";
    }

    m_phase = Phase::Result;
    showScreen(text, { "Continue" });
}

void Game::onResultContinue() {
    if (!m_party.isAlive()) {
        showEnding(m_party.causeOfDeath() + " Your journey ends on Day " +
                   std::to_string(m_trail.currentDay()) + ", " + partName(m_trail.currentPart()) + ".", true);
        return;
    }

    if (m_trail.isLastPartOfDay()) {
        showDaySummary();
    } else {
        m_trail.advance();
        showCurrentEvent();
    }
}

void Game::showDaySummary() {
    m_phase = Phase::DaySummary;
    showScreen("Day " + std::to_string(m_trail.currentDay()) + " is over. Food " +
               std::to_string(m_party.getFood()) + ", Health " + std::to_string(m_party.getHealth()) +
               ", Sanity " + std::to_string(m_party.getSanity()) + ", Wagon " +
               std::to_string(m_party.getWagon()) + ". The oxen are still watching.", { "Continue" });
}

void Game::onDaySummaryContinue() {
    if (!m_trail.hasDay(m_trail.currentDay() + 1)) {
        showEnding("The trail ends here, for now. You survived to the end of Day " +
                   std::to_string(m_trail.currentDay()) + ".", true);
        return;
    }
    m_trail.advance();
    showCurrentEvent();
}

void Game::showEnding(const std::string& text, bool canRestart) {
    m_phase = Phase::Ending;
    m_canRestart = canRestart;
    if (canRestart) {
        showScreen(text, { "Play again", "Quit" });
    } else {
        showScreen(text, { "Quit" });
    }
}

void Game::selectChoice(std::size_t index) {
    if (index >= m_buttons.size()) return;  // key pressed for a button this screen doesn't have

    switch (m_phase) {
    case Phase::Event:      onEventChoice(index);    break;
    case Phase::Result:     onResultContinue();      break;
    case Phase::DaySummary: onDaySummaryContinue();  break;
    case Phase::Ending:
        if (m_canRestart && index == 0) {
            startGame();
        } else {
            m_running = false;
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Bottom panel: text, buttons, mouse
// ---------------------------------------------------------------------------

void Game::showScreen(const std::string& text, const std::vector<std::string>& buttonLabels) {
    m_narrative = text;
    m_buttons.clear();

    // Rects are placeholders here; layoutButtons() positions them.
    for (std::size_t i = 0; i < buttonLabels.size() && i < kMaxChoices; ++i) {
        m_buttons.emplace_back(buttonLabels[i], SDL_FRect{ 0.0f, 0.0f, 0.0f, 0.0f });
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

void Game::drawStatBar(const char* label, int value, float y, SDL_Color fill) {
    // Label
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, label, kPadding, y, kStatScale);

    // Empty track
    SDL_FRect track{ kStatBarX, y, kStatBarWidth, kStatBarHeight };
    SDL_SetRenderDrawColor(m_renderer, 185, 165, 125, 255);
    SDL_RenderFillRect(m_renderer, &track);

    // Filled part, proportional to the stat (0..100)
    const float fraction = static_cast<float>(std::max(0, std::min(value, Party::kMaxStat))) /
                           static_cast<float>(Party::kMaxStat);
    SDL_FRect filled{ kStatBarX, y, kStatBarWidth * fraction, kStatBarHeight };
    SDL_SetRenderDrawColor(m_renderer, fill.r, fill.g, fill.b, 255);
    SDL_RenderFillRect(m_renderer, &filled);

    // Border and number
    SDL_SetRenderDrawColor(m_renderer, 90, 70, 50, 255);
    SDL_RenderRect(m_renderer, &track);
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, std::to_string(value), kStatBarX + kStatBarWidth + 12.0f, y, kStatScale);
}

void Game::renderTop() {
    SDL_FRect topRect;
    topRect.x = 0;
    topRect.y = 0;
    topRect.w = static_cast<float>(m_windowWidth);
    topRect.h = static_cast<float>(m_windowHeight) * kTopPanelRatio;

    SDL_SetRenderDrawColor(m_renderer, 210, 190, 150, 255); // parchment tan
    SDL_RenderFillRect(m_renderer, &topRect);

    // Which day and part of the day we are in
    const std::string heading = "Day " + std::to_string(m_trail.currentDay()) + " - " +
                                partName(m_trail.currentPart());
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, heading, kPadding, kPadding, kHeadingScale);

    // The party's four stats
    drawStatBar("Food",   m_party.getFood(),   84.0f,  SDL_Color{ 205, 150,  55, 255 });
    drawStatBar("Health", m_party.getHealth(), 120.0f, SDL_Color{ 190,  60,  50, 255 });
    drawStatBar("Sanity", m_party.getSanity(), 156.0f, SDL_Color{ 115,  90, 170, 255 });
    drawStatBar("Wagon",  m_party.getWagon(),  192.0f, SDL_Color{ 120,  90,  60, 255 });

    // TODO: wagon sprite, trail dots and party portraits go on the right-hand side.
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
    const float textTop   = topHeight + kPadding;
    const float available = m_buttonsTop - kTextGap - textTop;
    const int   maxLines  = std::max(1, static_cast<int>(available / m_text.lineHeight(kNarrativeScale)));

    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawWrapped(m_renderer, m_narrative,
                       kPadding, textTop,
                       static_cast<float>(m_windowWidth) - 2.0f * kPadding,
                       kNarrativeScale, maxLines);

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
