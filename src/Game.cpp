#include "Game.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
    const char* const kEventFile    = "data/events.txt";     // relative to the working directory
    const char* const kSettingsFile = "data/settings.txt";
    const char* const kAudioFolder  = "assets/audio";
    const char* const kLogFolder    = "playtest_logs";

    // Bottom panel layout (pixels)
    constexpr float       kPadding        = 24.0f;
    constexpr float       kButtonHeight   = 52.0f;
    constexpr float       kButtonGap      = 12.0f;
    constexpr float       kTextGap        = 12.0f;  // space between narrative text and buttons
    constexpr float       kNarrativeScale = 2.0f;   // 8px font drawn at 16px
    constexpr std::size_t kMaxChoices     = 4;      // matches keys 1-4 and the 2x2 button grid

    // Top panel layout
    constexpr float kHeadingScale  = 3.0f;
    constexpr float kStatScale     = 2.0f;
    constexpr float kStatBarX      = 140.0f;
    constexpr float kStatBarWidth  = 240.0f;
    constexpr float kStatBarHeight = 16.0f;
    constexpr SDL_FRect kSceneArea    { 470.0f, 24.0f, 466.0f, 252.0f };
    constexpr SDL_FRect kTitleArea    {  24.0f, 24.0f, 912.0f, 252.0f };
    constexpr SDL_FRect kProgressArea {  24.0f, 292.0f, 912.0f,  76.0f };

    // Family portraits (under the stat bars)
    constexpr float kPortraitSize = 56.0f;
    constexpr float kPortraitX    = 24.0f;
    constexpr float kPortraitStep = 100.0f;
    constexpr float kPortraitY    = 200.0f;

    std::string joinNames(const std::vector<std::string>& names) {
        if (names.empty()) return "nobody";
        std::string out;
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (i > 0) out += ", ";
            out += names[i];
        }
        return out;
    }
}

Game::Game() : m_narrator(NarrativeState::create(NarrativeKind::Stable)) {
    // The observers are registered here, once: from now on the party tells them about
    // every change and never needs to know what they do with it.
    m_party.addObserver(&m_thresholds);
    m_party.addObserver(&m_memberWatcher);
}

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

    // The scene is animated, so the loop redraws constantly. Vsync caps that at the
    // monitor's refresh rate instead of using a whole CPU core. (Not fatal if unsupported.)
    SDL_SetRenderVSync(m_renderer, 1);

    // Optional settings, then sound (silent if irrKlang or an audio device is missing)
    m_settings = Settings::load(kSettingsFile);
    for (const std::string& warning : m_settings.warnings) {
        std::cerr << "[settings] " << warning << "\n";
    }
    if (m_settings.audio) {
        m_audio.init(kAudioFolder);
    }

    const bool loaded = m_trail.loadFromFile(kEventFile);
    for (const std::string& warning : m_trail.getWarnings()) {
        std::cerr << "[events] " << warning << "\n";
    }

    if (loaded) {
        showTitle();
    } else {
        // Show the problem in the window instead of crashing or closing silently.
        showEnding(std::string("Could not load ") + kEventFile + ". Make sure a data folder "
                   "containing events.txt sits next to the game, then start it again. "
                   "Details are in the console window.", false, false);
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
            } else if (event.key.key == SDLK_M) {          // mute / unmute
                m_audio.setMuted(!m_audio.isMuted());
                if (m_phase == Phase::Title) showTitle();   // refresh the status line
            } else if (event.key.key == SDLK_L && m_phase == Phase::Title) {   // playtest log on/off
                m_settings.playtestLog = !m_settings.playtestLog;
                if (!m_settings.playtestLog) m_log.stop();
                showTitle();
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
    // (The scene animation is driven by the clock inside SceneRenderer.)
}

// ---------------------------------------------------------------------------
// Game flow: Title -> (Instructions) -> Event -> Result -> (next Event | DaySummary)
//            -> ... -> Ending -> (Play again -> Event)
// ---------------------------------------------------------------------------

void Game::showTitle() {
    m_phase = Phase::Title;
    m_audio.setMood(Mood::Calm);

    std::string status = "Sound: ";
    if (!m_audio.isAvailable())  status += "unavailable";
    else                         status += m_audio.isMuted() ? "OFF" : "ON";
    status += " (press M).  Playtest log: ";
    status += m_settings.playtestLog ? "ON, anonymous" : "OFF";
    status += " (press L).";

    showScreen("A journey west, with one family, many oxen, and an unreliable amount of luck. "
               "Choose carefully. Or don't.",
               { "Start journey", "How to play", "Quit" }, false, status);
}

void Game::showInstructions() {
    m_phase = Phase::Instructions;
    showScreen("Each day has five parts, and each part brings one event. Pick a choice with the mouse "
               "or the 1-4 keys. Watch Food, Health, Sanity, the Wagon and your family: the trail takes "
               "its toll every part. When your sanity slips the game starts to lie to you, but what "
               "happens to you is always true. M mutes the sound.",
               { "Start journey", "Back" });
}

void Game::startGame() {
    m_party.reset();
    m_trail.reset();
    m_thresholds.clearWarnings();
    m_memberWatcher.clearMessages();
    m_partsPlayed = 0;
    setNarrator(NarrativeKind::Stable);
    m_pendingNotice.clear();

    if (m_settings.playtestLog && !m_log.isOpen()) {
        const std::string label = std::string("built ") + __DATE__ + " " + __TIME__;
        if (m_log.start(kLogFolder, label)) {
            std::cout << "[playtest] logging to " << m_log.filePath() << "\n";
        } else {
            std::cerr << "[playtest] could not create a log file in " << kLogFolder << "\n";
        }
    }
    m_log.logRunStart();

    m_audio.play(Sfx::Start);
    showCurrentEvent();
}

void Game::showCurrentEvent() {
    m_activeEvent = m_trail.currentEvent(m_thresholds.active());

    if (!m_activeEvent) {
        // A gap in the data file. Don't crash: say which slot is empty and let play continue.
        m_phase = Phase::Result;
        showScreen("No event is written for Day " + std::to_string(m_trail.currentDay()) + ", " +
                   partName(m_trail.currentPart()) + ". Add one to " + kEventFile +
                   ". The day carries on without it.", { "Continue" });
        return;
    }

    std::cout << "[event] Day " << m_trail.currentDay() << ", " << partName(m_trail.currentPart())
              << (m_activeEvent->getCondition() ? std::string(" (desperate: ") + conditionName(*m_activeEvent->getCondition()) + ")"
                                                : std::string(""))
              << "\n";

    m_phase = Phase::Event;
    std::vector<std::string> labels;
    for (const Choice& choice : m_activeEvent->getChoices()) {
        labels.push_back(choice.getLabel());  // button labels always tell the truth
    }
    showScreen(m_activeEvent->getText(), labels, true);
}

void Game::onEventChoice(std::size_t index) {
    const Event* event = m_activeEvent;
    if (!event || index >= event->getChoices().size()) return;

    const Choice& choice = event->getChoices()[index];
    const Effects effects = choice.getEffects();

    m_party.apply(effects);
    const Effects upkeep = m_party.advanceTime();  // the trail's toll: food, sanity, wagon (and starvation)
    ++m_partsPlayed;

    const NarrativeKind before = m_narrator->kind();
    updateNarrator();                              // sanity may have crossed a threshold
    const bool narratorChanged = before != m_narrator->kind();

    std::cout << "Day " << m_trail.currentDay() << ", " << partName(m_trail.currentPart())
              << ": chose [" << (index + 1) << "] " << choice.getLabel()
              << " (" << effects.describe() << ") -> " << m_narrator->name() << "\n";

    // The story text may lie; the summary of what actually happened never does.
    std::string factual = "(" + effects.describe() + ") The trail takes its toll (" + upkeep.describe() + ").";
    if (m_party.getFood() == 0) {   // out of food: everyone is taking starvation damage
        factual += " The party is starving.";
    }
    const bool someoneDied = m_memberWatcher.sawDeath();
    for (const std::string& message : m_memberWatcher.takeMessages()) factual += " " + message;
    const std::vector<std::string> warnings = m_thresholds.takeWarnings();
    for (const std::string& warning : warnings) factual += " " + warning;

    // Sound: the strongest cue wins, then a change in the narrator's mood
    const int net = effects.food + effects.health + effects.sanity + effects.wagon;
    if (someoneDied)             m_audio.play(Sfx::Death);
    else if (!warnings.empty())  m_audio.play(Sfx::Warning);
    else                         m_audio.play(net > 0 ? Sfx::Good : Sfx::Bad);
    if (narratorChanged)         m_audio.play(Sfx::StateChange);

    m_log.logChoice(m_trail.currentDay(), partName(m_trail.currentPart()),
                    event->getCondition() ? conditionName(*event->getCondition()) : "normal",
                    static_cast<int>(index) + 1, choice.getLabel(),
                    m_party.getFood(), m_party.getHealth(), m_party.getSanity(), m_party.getWagon(),
                    m_narrator->name(), m_party.aliveCount());

    m_phase = Phase::Result;
    showScreen(choice.getResultText(), { "Continue" }, true, factual);
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
    m_audio.play(Sfx::DayEnd);
    // The recap is the moment of truth: real numbers, whatever state your mind is in.
    showScreen("Day " + std::to_string(m_trail.currentDay()) + " is over. The oxen are still watching.",
               { "Continue" }, false,
               "Food " + std::to_string(m_party.getFood()) + ", Health " + std::to_string(m_party.getHealth()) +
               ", Sanity " + std::to_string(m_party.getSanity()) + ", Wagon " +
               std::to_string(m_party.getWagon()) + ".");
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

std::string Game::journeySummary() const {
    std::vector<std::string> survivors;
    std::vector<std::string> lost;
    for (const PartyMember& member : m_party.getMembers()) {
        (member.isAlive() ? survivors : lost).push_back(member.getName());
    }

    std::string rank;
    switch (survivors.size()) {
    case 4:  rank = "The family is intact. Suspiciously."; break;
    case 3:  rank = "A small loss. The oxen send their regards."; break;
    case 2:  rank = "Half a family is still a family."; break;
    case 1:  rank = "One survivor, and a lot of explaining to do."; break;
    default: rank = "Nobody is left to tell the tale."; break;
    }

    const int totalParts = m_trail.lastDay() * kPartsPerDay;
    return "Survivors: " + joinNames(survivors) + ". Lost: " + joinNames(lost) + ". Food " +
           std::to_string(m_party.getFood()) + ", Health " + std::to_string(m_party.getHealth()) +
           ", Sanity " + std::to_string(m_party.getSanity()) + ", Wagon " + std::to_string(m_party.getWagon()) +
           ". Parts survived: " + std::to_string(m_partsPlayed) + " of " + std::to_string(totalParts) +
           ". " + rank;
}

void Game::showEnding(const std::string& text, bool canRestart, bool withSummary) {
    m_phase = Phase::Ending;
    m_canRestart = canRestart;
    setNarrator(NarrativeKind::Stable);  // the ending always tells the truth
    m_pendingNotice.clear();
    m_audio.setMood(Mood::Silent);

    if (withSummary) {
        m_audio.play(m_party.isAlive() ? Sfx::DayEnd : Sfx::Death);
        m_log.logRunEnd(m_party.isAlive() ? "survived" : m_party.causeOfDeath(), m_partsPlayed,
                        m_party.aliveCount(), m_party.getFood(), m_party.getHealth(),
                        m_party.getSanity(), m_party.getWagon());
    }

    const std::string summary = withSummary ? journeySummary() : "";
    if (canRestart) {
        showScreen(text, { "Play again", "Quit" }, false, summary);
    } else {
        showScreen(text, { "Quit" }, false, summary);
    }
}

void Game::selectChoice(std::size_t index) {
    if (index >= m_buttons.size()) return;  // key pressed for a button this screen doesn't have

    m_audio.play(Sfx::Click);
    switch (m_phase) {
    case Phase::Title:
        if (index == 0)      startGame();
        else if (index == 1) showInstructions();
        else                 m_running = false;
        break;
    case Phase::Instructions:
        if (index == 0) startGame();
        else            showTitle();
        break;
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
// Narrator (State pattern): sanity decides how much the game can be trusted
// ---------------------------------------------------------------------------

Mood Game::moodFor(NarrativeKind kind) {
    switch (kind) {
    case NarrativeKind::Unreliable: return Mood::Uneasy;
    case NarrativeKind::Breakdown:  return Mood::Dread;
    case NarrativeKind::Stable:     break;
    }
    return Mood::Calm;
}

void Game::setNarrator(NarrativeKind kind) {
    m_narrator = NarrativeState::create(kind);
    m_audio.setMood(moodFor(kind));  // the music follows the narrator
}

void Game::updateNarrator() {
    const NarrativeKind wanted = NarrativeState::kindForSanity(m_party.getSanity());
    if (wanted == m_narrator->kind()) return;

    setNarrator(wanted);
    m_pendingNotice = m_narrator->enterMessage();  // shown once, on the next screen
    std::cout << "[narrator] now " << m_narrator->name() << " (sanity " << m_party.getSanity() << ")\n";
}

// ---------------------------------------------------------------------------
// Bottom panel: text, buttons, mouse
// ---------------------------------------------------------------------------

void Game::showScreen(const std::string& narrative, const std::vector<std::string>& buttonLabels,
                      bool lie, const std::string& factual) {
    ++m_screenSeed;  // new screen = new lies; the same screen always shows the same ones

    std::string shown = lie ? m_narrator->filterText(narrative, m_screenSeed) : narrative;
    if (!factual.empty()) {
        shown += " " + factual;
    }
    if (!m_pendingNotice.empty()) {
        shown += " " + m_pendingNotice;
        m_pendingNotice.clear();
    }
    m_narrative = shown;

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

void Game::drawStatBar(const char* label, Stat stat, int actualValue, float y, SDL_Color fill) {
    // What the HUD shows may be a lie, depending on the narrator's state.
    const int shown = m_narrator->shownValue(actualValue, stat, m_screenSeed);

    // Label
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, label, kPadding, y, kStatScale);

    // Empty track
    SDL_FRect track{ kStatBarX, y, kStatBarWidth, kStatBarHeight };
    SDL_SetRenderDrawColor(m_renderer, 185, 165, 125, 255);
    SDL_RenderFillRect(m_renderer, &track);

    // Filled part, proportional to the shown value (0..100)
    const float fraction = static_cast<float>(std::max(0, std::min(shown, Party::kMaxStat))) /
                           static_cast<float>(Party::kMaxStat);
    SDL_FRect filled{ kStatBarX, y, kStatBarWidth * fraction, kStatBarHeight };
    SDL_SetRenderDrawColor(m_renderer, fill.r, fill.g, fill.b, 255);
    SDL_RenderFillRect(m_renderer, &filled);

    // Border and number
    SDL_SetRenderDrawColor(m_renderer, 90, 70, 50, 255);
    SDL_RenderRect(m_renderer, &track);
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, m_narrator->valueLabel(shown), kStatBarX + kStatBarWidth + 12.0f, y, kStatScale);
}

void Game::drawMemberCard(std::size_t index, float x) {
    const PartyMember& member = m_party.getMembers()[index];

    const SDL_FRect portrait{ x, kPortraitY, kPortraitSize, kPortraitSize };
    m_scene.drawPortrait(m_renderer, portrait, static_cast<int>(index), member.getStatus());

    // Little health bar. It is part of the HUD, so it lies like the rest (but the dead stay empty).
    const int shown = member.isAlive()
        ? m_narrator->shownValue(member.getHealth(), Stat::Health, m_screenSeed + 17U * static_cast<unsigned>(index + 1))
        : 0;
    const float fraction = static_cast<float>(std::max(0, std::min(shown, 100))) / 100.0f;
    const SDL_FRect track{ x, kPortraitY + kPortraitSize + 4.0f, kPortraitSize, 6.0f };
    SDL_SetRenderDrawColor(m_renderer, 185, 165, 125, 255);
    SDL_RenderFillRect(m_renderer, &track);
    const SDL_FRect filled{ x, track.y, kPortraitSize * fraction, track.h };
    SDL_SetRenderDrawColor(m_renderer, 190, 60, 50, 255);
    SDL_RenderFillRect(m_renderer, &filled);
    SDL_SetRenderDrawColor(m_renderer, 90, 70, 50, 255);
    SDL_RenderRect(m_renderer, &track);

    // Name, centred under the bar
    const std::string& name = member.getName();
    const float nameX = x + (kPortraitSize - m_text.textWidth(name, 1.0f)) * 0.5f;
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, name, nameX, track.y + 10.0f, 1.0f);
}

void Game::renderHud() {
    // Which day and part of the day we are in
    const std::string heading = "Day " + std::to_string(m_trail.currentDay()) + " - " +
                                partName(m_trail.currentPart());
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, heading, kPadding, kPadding, kHeadingScale);

    // The party's four stats
    drawStatBar("Food",   Stat::Food,   m_party.getFood(),   76.0f,  SDL_Color{ 205, 150,  55, 255 });
    drawStatBar("Health", Stat::Health, m_party.getHealth(), 108.0f, SDL_Color{ 190,  60,  50, 255 });
    drawStatBar("Sanity", Stat::Sanity, m_party.getSanity(), 140.0f, SDL_Color{ 115,  90, 170, 255 });
    drawStatBar("Wagon",  Stat::Wagon,  m_party.getWagon(),  172.0f, SDL_Color{ 120,  90,  60, 255 });

    // The family
    for (std::size_t i = 0; i < m_party.getMembers().size(); ++i) {
        drawMemberCard(i, kPortraitX + kPortraitStep * static_cast<float>(i));
    }

    // The wagon and its oxen, under a sky that follows the time of day
    m_scene.drawScene(m_renderer, kSceneArea, m_trail.currentPart(), SDL_GetTicks(), m_narrator->dread());

    // How far along the trail we are
    m_scene.drawProgress(m_renderer, m_text, kProgressArea, m_trail.lastDay(),
                         m_trail.currentDay(), m_trail.currentPart());
}

void Game::renderTitleArt() {
    // The wagon drives across a sky that cycles through the day
    const Uint64 now = SDL_GetTicks();
    const TimePart part = static_cast<TimePart>((now / 4000) % kPartsPerDay);
    const float travel = static_cast<float>(now % 14000) / 14000.0f;
    m_scene.drawScene(m_renderer, kTitleArea, part, now, 0.0f, travel);

    const std::string title = "THE SCARYGON TRAIL";
    const std::string subtitle = "A journey west. What could go wrong?";
    const float titleX = kTitleArea.x + (kTitleArea.w - m_text.textWidth(title, 4.0f)) * 0.5f;
    const float subX   = kTitleArea.x + (kTitleArea.w - m_text.textWidth(subtitle, 2.0f)) * 0.5f;

    SDL_SetRenderDrawColor(m_renderer, 40, 25, 20, 255);             // shadow
    m_text.drawLine(m_renderer, title, titleX + 3.0f, 43.0f, 4.0f);
    m_text.drawLine(m_renderer, subtitle, subX + 2.0f, 92.0f, 2.0f);
    SDL_SetRenderDrawColor(m_renderer, 250, 236, 200, 255);          // text
    m_text.drawLine(m_renderer, title, titleX, 40.0f, 4.0f);
    m_text.drawLine(m_renderer, subtitle, subX, 90.0f, 2.0f);
}

void Game::renderTop() {
    SDL_FRect topRect;
    topRect.x = 0;
    topRect.y = 0;
    topRect.w = static_cast<float>(m_windowWidth);
    topRect.h = static_cast<float>(m_windowHeight) * kTopPanelRatio;

    SDL_SetRenderDrawColor(m_renderer, 210, 190, 150, 255); // parchment tan
    SDL_RenderFillRect(m_renderer, &topRect);

    if (m_phase == Phase::Title || m_phase == Phase::Instructions) {
        renderTitleArt();
    } else {
        renderHud();
    }
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
    m_log.stop();
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
