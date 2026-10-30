#include "Game.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>

namespace {
    const char* const kEventFile    = "data/events.txt";    
    const char* const kSettingsFile = "data/settings.txt";
    const char* const kProgressFile = "save/progress.txt";
    const char* const kAudioFolder  = "assets/audio";
    const char* const kLogFolder    = "playtest_logs";

    // This part of the code is for the bottom panel layout (pixels)
    constexpr float       kPadding        = 24.0f;
    constexpr float       kButtonHeight   = 52.0f;
    constexpr float       kButtonGap      = 12.0f;
    constexpr float       kTextGap        = 12.0f;  // This part of the code makes space between narrative text and buttons
    constexpr float       kNarrativeScale = 2.0f;   // This part of the code sets 8px font drawn at 16px
    constexpr std::size_t kMaxChoices     = 4;      // This part of the code matches keys 1-4 and the 2x2 button grid

    // This part of the code is for the top panel layout
    constexpr float kHeadingScale  = 3.0f;
    constexpr float kStatScale     = 2.0f;
    constexpr float kStatBarX      = 140.0f;
    constexpr float kStatBarWidth  = 240.0f;
    constexpr float kStatBarHeight = 16.0f;
    constexpr SDL_FRect kSceneArea    { 470.0f, 24.0f, 466.0f, 252.0f };
    constexpr SDL_FRect kTitleArea    {  24.0f, 24.0f, 912.0f, 252.0f };
    constexpr SDL_FRect kProgressArea {  24.0f, 292.0f, 912.0f,  76.0f };

    // This part of the code is for the family portraits (under the stat bars)
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
    // This part of the code is for the observers to be registered, once: from now on the party tells them about
    // every change and never needs to know what they do with it.
    m_party.addObserver(&m_thresholds);
    m_party.addObserver(&m_memberWatcher);
}

Game::~Game() {
    shutdown();
}

bool Game::init(const char* title, int width, int height) {
    // This part of the code is for the SDL3 change from SDL2: SDL_Init returns bool (true = success),
    // not an int where 0 = success. A mistake if 
    // older SDL2 tutorials are followed instead.
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    m_windowWidth  = width;
    m_windowHeight = height;

    // This part of the code is for the SDL3 change: SDL_CreateWindow no longer takes x/y position
    // arguments (SDL2 had them before flags). Position is set
    // separately if needed, or just let the OS place it.
    m_window = SDL_CreateWindow(title, width, height, 0);
    if (!m_window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    // This part of the code is for the SDL3 change: SDL_CreateRenderer takes (window, driver_name)
    // where driver_name is usually just nullptr to let SDL pick.
    // SDL2 had an extra integer flags parameter here.
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    // This part of the code is for the animated scene, so the loop redraws constantly. Vsync caps that at the
    // monitor's refresh rate instead of using a whole CPU core. (Not fatal if unsupported.)
    SDL_SetRenderVSync(m_renderer, 1);

    // This part of the code is for the optional settings, then sound (silent if irrKlang or an audio device is missing)
    m_settings = Settings::load(kSettingsFile);
    for (const std::string& warning : m_settings.warnings) {
        std::cerr << "[settings] " << warning << "\n";
    }
    if (m_settings.audio) {
        m_audio.init(kAudioFolder);
    }

    std::vector<std::string> progressWarnings;
    m_progress = Progress::load(kProgressFile, &progressWarnings);
    for (const std::string& warning : progressWarnings) std::cerr << "[progress] " << warning << "\n";
    m_party.setDifficulty(m_progress.difficulty);

    const bool loaded = m_trail.loadFromFile(kEventFile);
    for (const std::string& warning : m_trail.getWarnings()) {
        std::cerr << "[events] " << warning << "\n";
    }

    if (loaded) {
        showTitle();
    } else {
        // This part of the code shows the problem in the window instead of crashing or closing silently.
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
        // This part of the code is for the SDL3 change: event type constants are now SDL_EVENT_* instead
        // of SDL_* (e.g. SDL_EVENT_QUIT instead of SDL_QUIT).
        switch (event.type) {
        case SDL_EVENT_QUIT:
            m_running = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.repeat) break;  // This part of the code will ignore held-down key repeats
            if (event.key.key == SDLK_ESCAPE) {
                m_running = false;
            } else if (event.key.key >= SDLK_1 && event.key.key <= SDLK_4) {
                selectChoice(static_cast<std::size_t>(event.key.key - SDLK_1));
            } else if (event.key.key == SDLK_M) {          // This part of the code is for the mute and unmute
                m_audio.setMuted(!m_audio.isMuted());
                if (m_phase == Phase::Title) showTitle();   // This part of the code refreshes the status line
            } else if (event.key.key == SDLK_L && m_phase == Phase::Title) {   // This part of the code for the playtest log on or off
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
    // This part of the code makes it so nothing runs on a timer: the game advances when the player picks a button.
    // (The scene animation is driven by the clock inside SceneRenderer.)
}

// -----------------------------------------------------------------------------------------
// Average ame flow: Title -> (Instructions) -> Event -> Result -> (next Event | DaySummary)
//            -> ... -> Ending -> (Play again -> Event)
// -----------------------------------------------------------------------------------------

void Game::showTitle() {
    m_phase = Phase::Title;
    m_audio.setMood(Mood::Calm);
    m_stage = SceneRenderer::Stage();

    std::string status = "Seen: " + std::to_string(m_progress.animationsSeen()) + "/" +
                         std::to_string(Progress::animationTotal()) + " animations, " +
                         std::to_string(m_progress.endingsSeen()) + "/" + std::to_string(kEndingCount) + " endings.";
    if (m_progress.clearedCount() > 0) {
        status += " Cleared:";
        for (Difficulty d : { Difficulty::Easy, Difficulty::Normal, Difficulty::Hard }) {
            if (m_progress.cleared(d)) status += std::string(" ") + difficultyName(d);
        }
        status += ".";
    }
    status += "  Sound: ";
    if (!m_audio.isAvailable())  status += "unavailable";
    else                         status += m_audio.isMuted() ? "OFF" : "ON";
    status += " (M). Log: ";
    status += m_settings.playtestLog ? "ON" : "OFF";
    status += " (L).";

    showScreen("A journey west, with one family, many oxen, and an unreliable amount of luck. "
               "Choose carefully. Or don't.",
               { "Start journey", std::string("Difficulty: ") + difficultyName(m_progress.difficulty),
                 "How to play", "Quit" }, false, status);
}

void Game::cycleDifficulty() {
    m_progress.difficulty = nextDifficulty(m_progress.difficulty);
    m_party.setDifficulty(m_progress.difficulty);
    saveProgress();
    showTitle();
}

void Game::saveProgress() {
    if (!m_progress.save(kProgressFile)) {
        static bool told = false;   // This part of the code makes the game say it once: progress then only lasts until the game is closed
        if (!told) { std::cerr << "[progress] could not write " << kProgressFile << "; progress will not be remembered\n"; told = true; }
    }
}

void Game::showInstructions() {
    m_phase = Phase::Instructions;
    showScreen("Each day has five parts, each with one event. Choose with the mouse or keys 1-4. Watch "
               "Food, Health, Sanity, the Wagon and your family: the trail takes its toll every part. "
               "The NARRATOR meter shows how far to trust the text. When sanity slips the game lies, but "
               "the (bracketed) summary and the day recap are always true. Pick Easy, Normal or Hard on the title screen. "
               "Every scene, reaction and ending you find is remembered, so there is always something new to see. M mutes.",
               { "Start journey", "Back" });
}

void Game::startGame() {
    m_party.setDifficulty(m_progress.difficulty);
    m_party.reset();
    m_trail.reset();
    m_thresholds.clearWarnings();
    m_memberWatcher.clearMessages();
    m_partsPlayed = 0;
    setNarrator(NarrativeKind::Stable);
    m_shiftTo.reset();
    m_lies.reset();
    m_stage = SceneRenderer::Stage();
    m_newDiscoveries = 0;
    m_ending.reset();
    m_endingIsNew = false;
    ++m_progress.runs;
    saveProgress();

    if (m_settings.playtestLog && !m_log.isOpen()) {
        const std::string label = std::string("built ") + __DATE__ + " " + __TIME__;
        if (m_log.start(kLogFolder, label)) {
            std::cout << "[playtest] logging to " << m_log.filePath() << "\n";
        } else {
            std::cerr << "[playtest] could not create a log file in " << kLogFolder << "\n";
        }
    }
    m_log.logRunStart(std::string("new run, difficulty ") + difficultyName(m_party.difficulty()));

    m_audio.play(Sfx::Start);
    showCurrentEvent();
}

void Game::showCurrentEvent() {
    m_activeEvent = m_trail.currentEvent(m_thresholds.active());

    if (!m_activeEvent) {
    
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
    m_stage.scene = m_activeEvent->getScene();
    m_stage.action = ActionId::None;
    if (m_progress.markScene(m_stage.scene)) ++m_newDiscoveries;
    std::vector<std::string> labels;
    for (const Choice& choice : m_activeEvent->getChoices()) {
        labels.push_back(choice.getLabel());  // This part of the code is for the button labels to always tell the truth
    }
    showScreen(m_activeEvent->getText(), labels, true);
}

void Game::onEventChoice(std::size_t index) {
    const Event* event = m_activeEvent;
    if (!event || index >= event->getChoices().size()) return;

    const Choice& choice = event->getChoices()[index];
    const Effects effects = m_party.adjusted(choice.getEffects());   // This part of the code makes the Easy / Normal / Hard scale what choices do

    m_party.apply(effects);
    const Effects upkeep = m_party.advanceTime();  // This part of the code is for the trail's toll: food, sanity, wagon (and starvation)
    ++m_partsPlayed;

    // This part of the code checks if sanity crossed a threshold the narrator does NOT change mid-screen. This result is still
    // shown by the old narrator, then a screen of its own announces the change (before any lies).
    const NarrativeKind wanted = NarrativeState::kindForSanity(m_party.getSanity());
    m_shiftTo.reset();
    if (wanted != m_narrator->kind()) m_shiftTo = wanted;

    std::cout << "Day " << m_trail.currentDay() << ", " << partName(m_trail.currentPart())
              << ": chose [" << (index + 1) << "] " << choice.getLabel()
              << " (" << effects.describe() << ") -> " << m_narrator->name() << "\n";

    // This part of the code will make the story text lie; the summary of what actually happened never does.
    std::string factual = "(" + effects.describe() + ") The trail takes its toll (" + upkeep.describe() + ").";
    if (m_party.getFood() == 0) {   // This part of the code is checks out of food: activates everyone is taking starvation damage.
        factual += " The party is starving.";
    }
    const bool someoneDied = m_memberWatcher.sawDeath();
    for (const std::string& message : m_memberWatcher.takeMessages()) factual += " " + message;
    const std::vector<std::string> warnings = m_thresholds.takeWarnings();
    for (const std::string& warning : warnings) factual += " " + warning;

    // This part of the code is for sound: the strongest cue wins, then a change in the narrator's mood
    const int net = effects.food + effects.health + effects.sanity + effects.wagon;
    if (someoneDied)             m_audio.play(Sfx::Death);
    else if (!warnings.empty())  m_audio.play(Sfx::Warning);
    else                         m_audio.play(net > 0 ? Sfx::Good : Sfx::Bad);

    m_log.logChoice(m_trail.currentDay(), partName(m_trail.currentPart()),
                    event->getCondition() ? conditionName(*event->getCondition()) : "normal",
                    static_cast<int>(index) + 1, choice.getLabel(),
                    m_party.getFood(), m_party.getHealth(), m_party.getSanity(), m_party.getWagon(),
                    m_narrator->name(), m_party.aliveCount());

    // This part of the code checks how the family reacts: the top picture plays the animation that goes with this choice.
    m_stage.action = choice.getAction();
    m_stage.outcome = net > 0 ? 1 : (net < 0 ? -1 : 0);
    m_stageStartMs = SDL_GetTicks();
    if (m_progress.markAction(m_stage.action)) ++m_newDiscoveries;

    m_phase = Phase::Result;
    showScreen(choice.getResultText(), { "Continue" }, true, factual);
}

void Game::onResultContinue() {
    if (!m_party.isAlive()) {
        showEnding(m_party.causeOfDeath() + " Your journey ends on Day " +
                   std::to_string(m_trail.currentDay()) + ", " + partName(m_trail.currentPart()) + ".", true);
        return;
    }

    if (m_shiftTo) {            // This part of the code is for the narrator's state changed: says so, on its own screen
        showNarratorShift();
        return;
    }
    proceedAfterResult();
}

void Game::proceedAfterResult() {
    if (m_trail.isLastPartOfDay()) {
        showDaySummary();
    } else {
        m_trail.advance();
        showCurrentEvent();
    }
}

void Game::showNarratorShift() {
    const NarrativeKind kind = *m_shiftTo;
    m_shiftTo.reset();
    setNarrator(kind);          // This part of the code is for the music and the trust meter change, before any lying starts
    m_audio.play(Sfx::StateChange);
    std::cout << "[narrator] now " << m_narrator->name() << " (sanity " << m_party.getSanity() << ")\n";

    std::string text;
    switch (kind) {
    case NarrativeKind::Unreliable:
        text = "The narrator is starting to slip. From now on it may swap words and the bars may be wrong. "
               "The summary in brackets after each result, and the recap at the end of each day, are always true.";
        break;
    case NarrativeKind::Breakdown:
        text = "The narrator has broken down. Expect wrong words, wrong bars, numbers hidden behind question "
               "marks and comments nobody asked for. The brackets after each result and the day recap are still true.";
        break;
    case NarrativeKind::Stable:
        text = "The narrator has recovered. For now, what it tells you is true.";
        break;
    }

    m_phase = Phase::NarratorShift;
    showScreen(text, { "Continue" });
}

void Game::onNarratorShiftContinue() {
    proceedAfterResult();
}

void Game::showDaySummary() {
    m_phase = Phase::DaySummary;
    m_audio.play(Sfx::DayEnd);
    // This part of the code is for making the  recap the moment of truth: real numbers, whatever state your mind is in.
    showScreen("Day " + std::to_string(m_trail.currentDay()) + " is over. The oxen are still watching.",
               { "Continue" }, false,
               "Food " + std::to_string(m_party.getFood()) + ", Health " + std::to_string(m_party.getHealth()) +
               ", Sanity " + std::to_string(m_party.getSanity()) + ", Wagon " +
               std::to_string(m_party.getWagon()) + ". " + m_lies.dayReport());
}

void Game::onDaySummaryContinue() {
    if (!m_trail.hasDay(m_trail.currentDay() + 1)) {
        showEnding("The trail ends here, for now. You survived to the end of Day " +
                   std::to_string(m_trail.currentDay()) + ".", true);
        return;
    }
    m_lies.startDay();   // This part of the code is making tomorrow's truth check start from zero
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
           ". " + rank + (m_lies.totalCount() > 0 ? " " + m_lies.journeyReport() : std::string());
}

void Game::showEnding(const std::string& text, bool canRestart, bool withSummary) {
    m_phase = Phase::Ending;
    m_canRestart = canRestart;
    setNarrator(NarrativeKind::Stable);  // This part of the code is makes the ending always tells the truth
    m_shiftTo.reset();
    m_audio.setMood(Mood::Silent);

    if (withSummary) {
        m_audio.play(m_party.isAlive() ? Sfx::DayEnd : Sfx::Death);
        m_log.logRunEnd(m_party.isAlive() ? "survived" : m_party.causeOfDeath(), m_partsPlayed,
                        m_party.aliveCount(), m_party.getFood(), m_party.getHealth(),
                        m_party.getSanity(), m_party.getWagon());

        // This part of the code is remembers how it ended: every different ending is something new to find.
        const EndingId ending = Progress::classify(m_party.isAlive(), m_party.aliveCount(),
                                                   m_party.getSanity(), m_party.getWagon());
        m_ending = ending;
        m_endingIsNew = m_progress.markEnding(ending);
        if (m_party.isAlive()) m_progress.markCleared(m_party.difficulty());
        saveProgress();
    } else {
        m_ending.reset();
    }
    m_stage = SceneRenderer::Stage();  

    const std::string summary = withSummary ? journeySummary() : "";
    if (canRestart) {
        showScreen(text, { "Play again", "Change difficulty", "Quit" }, false, summary);
    } else {
        showScreen(text, { "Quit" }, false, summary);
    }
}

void Game::selectChoice(std::size_t index) {
    if (index >= m_buttons.size()) return; 

    m_audio.play(Sfx::Click);
    switch (m_phase) {
    case Phase::Title:
        if (index == 0)      startGame();
        else if (index == 1) cycleDifficulty();
        else if (index == 2) showInstructions();
        else                 m_running = false;
        break;
    case Phase::Instructions:
        if (index == 0) startGame();
        else            showTitle();
        break;
    case Phase::Event:      onEventChoice(index);    break;
    case Phase::Result:     onResultContinue();      break;
    case Phase::NarratorShift: onNarratorShiftContinue(); break;
    case Phase::DaySummary: onDaySummaryContinue();  break;
    case Phase::Ending:
        if (m_canRestart && index == 0)      startGame();
        else if (m_canRestart && index == 1) showTitle();      // This part of the code is for changing the difficulty, then start again
        else                                 m_running = false;
        break;
    }
}

// Narrator (State pattern): sanity decides how much the game can be trusted

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
    m_audio.setMood(moodFor(kind));  // This part of the code is so the music follows the narrator
}

// This part of the code is for the Bottom panel: text, buttons, mouse

void Game::showScreen(const std::string& narrative, const std::vector<std::string>& buttonLabels,
                      bool lie, const std::string& factual) {
    ++m_screenSeed;  // This part of the code makes new screen = new lies; the same screen always shows the same ones

    std::string shown = lie ? m_narrator->filterText(narrative, m_screenSeed) : narrative;
    if (lie) m_lies.recordText(narrative, shown);   // This part of the code is keeping score: it is revealed in the day's truth check

    // This part of the code makes the HUD bars lie too, on the screens that show them (the day recap always shows the truth)
    if (m_phase == Phase::Event || m_phase == Phase::Result) {
        const struct { const char* name; Stat stat; int truth; } bars[] = {
            { "Food",   Stat::Food,   m_party.getFood()   },
            { "Health", Stat::Health, m_party.getHealth() },
            { "Sanity", Stat::Sanity, m_party.getSanity() },
            { "Wagon",  Stat::Wagon,  m_party.getWagon()  },
        };
        std::vector<LieTracker::Bar> readings;
        for (const auto& bar : bars) {
            readings.push_back({ bar.name, m_narrator->shownValue(bar.truth, bar.stat, m_screenSeed), bar.truth });
        }
        m_lies.recordBars(readings);
    }

    if (!factual.empty()) {
        shown += " " + factual;
    }
    m_narrative = shown;

    m_buttons.clear();
  
    for (std::size_t i = 0; i < buttonLabels.size() && i < kMaxChoices; ++i) {
        m_buttons.emplace_back(buttonLabels[i], SDL_FRect{ 0.0f, 0.0f, 0.0f, 0.0f });
    }
    layoutButtons();

    // This part of the code makes it so refresh hover state so a button appearing under the cursor lights up immediately.
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    SDL_GetMouseState(&mouseX, &mouseY);
    handleMouseMotion(mouseX, mouseY);
}

void Game::layoutButtons() {
    // This part of the code makes the buttons sit in a 2-column grid anchored to the bottom of the window,
    // This part of the code is here so the narrative text above gets whatever room is left.
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

// This section of the code is for rendering

void Game::drawStatBar(const char* label, Stat stat, int actualValue, float y, SDL_Color fill) {
    // This part of the code checks Wwat the HUD shows may be a lie, depending on the narrator's state.
    // The day recap is the moment of truth: the bars show the real numbers.
    const bool truth = m_phase == Phase::DaySummary || m_phase == Phase::Ending;
    const int shown = truth ? actualValue : m_narrator->shownValue(actualValue, stat, m_screenSeed);

    // This part of the code is for a label
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, label, kPadding, y, kStatScale);

    // This part of the code is for an empty track
    SDL_FRect track{ kStatBarX, y, kStatBarWidth, kStatBarHeight };
    SDL_SetRenderDrawColor(m_renderer, 185, 165, 125, 255);
    SDL_RenderFillRect(m_renderer, &track);

    // This part of the code is for a filled part, proportional to the shown value (0..100)
    const float fraction = static_cast<float>(std::max(0, std::min(shown, Party::kMaxStat))) /
                           static_cast<float>(Party::kMaxStat);
    SDL_FRect filled{ kStatBarX, y, kStatBarWidth * fraction, kStatBarHeight };
    SDL_SetRenderDrawColor(m_renderer, fill.r, fill.g, fill.b, 255);
    SDL_RenderFillRect(m_renderer, &filled);

    // This part of the code is for the border and number
    SDL_SetRenderDrawColor(m_renderer, 90, 70, 50, 255);
    SDL_RenderRect(m_renderer, &track);
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, truth ? std::to_string(shown) : m_narrator->valueLabel(shown),
                    kStatBarX + kStatBarWidth + 12.0f, y, kStatScale);
}

void Game::drawMemberCard(std::size_t index, float x) {
    const PartyMember& member = m_party.getMembers()[index];

    const SDL_FRect portrait{ x, kPortraitY, kPortraitSize, kPortraitSize };
    m_scene.drawPortrait(m_renderer, portrait, static_cast<int>(index), member.getStatus());

    // This part of the code is for the little health bar. It is part of the HUD, so it lies like the rest (but the dead stay empty).
    const bool truth = m_phase == Phase::DaySummary || m_phase == Phase::Ending;
    const int shown = !member.isAlive() ? 0
        : truth ? member.getHealth()
                : m_narrator->shownValue(member.getHealth(), Stat::Health, m_screenSeed + 17U * static_cast<unsigned>(index + 1));
    const float fraction = static_cast<float>(std::max(0, std::min(shown, 100))) / 100.0f;
    const SDL_FRect track{ x, kPortraitY + kPortraitSize + 4.0f, kPortraitSize, 6.0f };
    SDL_SetRenderDrawColor(m_renderer, 185, 165, 125, 255);
    SDL_RenderFillRect(m_renderer, &track);
    const SDL_FRect filled{ x, track.y, kPortraitSize * fraction, track.h };
    SDL_SetRenderDrawColor(m_renderer, 190, 60, 50, 255);
    SDL_RenderFillRect(m_renderer, &filled);
    SDL_SetRenderDrawColor(m_renderer, 90, 70, 50, 255);
    SDL_RenderRect(m_renderer, &track);

    // This part of the code makes the name, centred under the bar
    const std::string& name = member.getName();
    const float nameX = x + (kPortraitSize - m_text.textWidth(name, 1.0f)) * 0.5f;
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, name, nameX, track.y + 10.0f, 1.0f);
}

void Game::renderHud() {
    // This part of the code checks which day and part of the day we are in
    const std::string heading = "Day " + std::to_string(m_trail.currentDay()) + " - " +
                                partName(m_trail.currentPart());
    SDL_SetRenderDrawColor(m_renderer, 60, 40, 30, 255);
    m_text.drawLine(m_renderer, heading, kPadding, kPadding, kHeadingScale);

    // This part of the code is checks how far to trust what the game says right now
    drawNarratorMeter();

    // This part of the code is for the party's four stats
    drawStatBar("Food",   Stat::Food,   m_party.getFood(),   76.0f,  SDL_Color{ 205, 150,  55, 255 });
    drawStatBar("Health", Stat::Health, m_party.getHealth(), 108.0f, SDL_Color{ 190,  60,  50, 255 });
    drawStatBar("Sanity", Stat::Sanity, m_party.getSanity(), 140.0f, SDL_Color{ 115,  90, 170, 255 });
    drawStatBar("Wagon",  Stat::Wagon,  m_party.getWagon(),  172.0f, SDL_Color{ 120,  90,  60, 255 });

    for (std::size_t i = 0; i < m_party.getMembers().size(); ++i) {
        drawMemberCard(i, kPortraitX + kPortraitStep * static_cast<float>(i));
    }

    // This part of the code is for the road: the event's scene while you decide, the family's reaction once you have, under a sky
    // that follows the time of day.
    SceneRenderer::Stage stage = m_stage;
    if (m_phase == Phase::Result) {
        stage.actionSeconds = static_cast<float>(SDL_GetTicks() - m_stageStartMs) / 1000.0f;
    } else if (m_phase == Phase::Event) {
        stage.action = ActionId::None;
    } else {
        stage = SceneRenderer::Stage();   // This part of the code checks narrator shifts, recaps and endings: just the road
    }
    m_scene.drawStage(m_renderer, m_text, kSceneArea, m_trail.currentPart(), SDL_GetTicks(),
                      m_narrator->dread(), stage);

    // This part of the code checks how far along the trail the player is 
    m_scene.drawProgress(m_renderer, m_text, kProgressArea, m_trail.lastDay(),
                         m_trail.currentDay(), m_trail.currentPart());
}

void Game::drawNarratorMeter() {
    // This part of the code is makes sure a label that is always on screen, so the player can see how far to trust the text.
    const char* label = "NARRATOR: TRUSTWORTHY";
    SDL_Color color{ 70, 130, 70, 255 };
    switch (m_narrator->kind()) {
    case NarrativeKind::Unreliable: label = "NARRATOR: SHAKY";  color = SDL_Color{ 200, 140,  40, 255 }; break;
    case NarrativeKind::Breakdown:  label = "NARRATOR: LYING";  color = SDL_Color{ 190,  60,  50, 255 }; break;
    case NarrativeKind::Stable:     break;
    }
    if (m_phase == Phase::DaySummary || m_phase == Phase::Ending) {
        label = "NARRATOR: TRUTH CHECK";
        color = SDL_Color{ 70, 130, 70, 255 };
    }

    constexpr float scale = 1.5f;
    const float width = m_text.textWidth(label, scale) + 20.0f;
    const SDL_FRect pill{ kPadding, 50.0f, width, 20.0f };
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(m_renderer, &pill);
    SDL_SetRenderDrawColor(m_renderer, 245, 240, 225, 255);
    m_text.drawLine(m_renderer, label, pill.x + 10.0f, pill.y + 4.0f, scale);
    drawDifficultyTag(pill.x + width + 8.0f);
}

void Game::drawDifficultyTag(float x) {
    const char* name = difficultyName(m_party.difficulty());
    std::string label = name;
    for (char& c : label) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    constexpr float scale = 1.5f;
    const SDL_FRect tag{ x, 50.0f, m_text.textWidth(label, scale) + 16.0f, 20.0f };
    SDL_SetRenderDrawColor(m_renderer, 90, 70, 50, 255);
    SDL_RenderFillRect(m_renderer, &tag);
    SDL_SetRenderDrawColor(m_renderer, 245, 240, 225, 255);
    m_text.drawLine(m_renderer, label, tag.x + 8.0f, tag.y + 4.0f, scale);
}

void Game::drawNarratorBanner() {
    // This part of the code makes it drawn over the scene when the narrator's state changes, so it cannot be missed.
    const char* top = "THE NARRATOR";
    const char* bottom = "HAS RECOVERED";
    SDL_Color color{ 70, 130, 70, 255 };
    switch (m_narrator->kind()) {
    case NarrativeKind::Unreliable: bottom = "IS SLIPPING";     color = SDL_Color{ 200, 140,  40, 255 }; break;
    case NarrativeKind::Breakdown:  bottom = "HAS BROKEN DOWN"; color = SDL_Color{ 190,  60,  50, 255 }; break;
    case NarrativeKind::Stable:     break;
    }

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, 215);
    SDL_RenderFillRect(m_renderer, &kSceneArea);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

    constexpr float scale = 3.0f;
    SDL_SetRenderDrawColor(m_renderer, 255, 250, 235, 255);
    const float centre = kSceneArea.x + kSceneArea.w * 0.5f;
    const float middle = kSceneArea.y + kSceneArea.h * 0.5f;
    m_text.drawLine(m_renderer, top,    centre - m_text.textWidth(top, scale) * 0.5f,    middle - 34.0f, scale);
    m_text.drawLine(m_renderer, bottom, centre - m_text.textWidth(bottom, scale) * 0.5f, middle + 6.0f,  scale);
}

void Game::drawEndingPlaque() {
    // This part of the code goes over the road at the end of a journey: how it ended, and how much is left to find.
    if (!m_ending) return;
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, 30, 22, 18, 205);
    SDL_RenderFillRect(m_renderer, &kSceneArea);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

    const float centre = kSceneArea.x + kSceneArea.w * 0.5f;
    float y = kSceneArea.y + 22.0f;
    auto line = [&](const std::string& text, float scale, SDL_Color color) {
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, 255);
        m_text.drawLine(m_renderer, text, centre - m_text.textWidth(text, scale) * 0.5f, y, scale);
        y += m_text.lineHeight(scale) + 6.0f;
    };
    line(m_endingIsNew ? "NEW ENDING FOUND!" : "ENDING", 2.5f, m_endingIsNew ? SDL_Color{ 255, 220, 90, 255 } : SDL_Color{ 235, 225, 200, 255 });
    line(endingTitle(*m_ending), 1.5f, SDL_Color{ 255, 250, 235, 255 });
    y += 6.0f;
    line("Endings found: " + std::to_string(m_progress.endingsSeen()) + " of " + std::to_string(kEndingCount), 1.5f, SDL_Color{ 220, 210, 190, 255 });
    line("Animations seen: " + std::to_string(m_progress.animationsSeen()) + " of " + std::to_string(Progress::animationTotal()), 1.5f, SDL_Color{ 220, 210, 190, 255 });
    if (m_newDiscoveries > 0) line(std::to_string(m_newDiscoveries) + " new this journey", 1.5f, SDL_Color{ 255, 220, 90, 255 });
    if (m_party.isAlive()) {
        line(std::string(difficultyName(m_party.difficulty())) + " cleared!", 1.5f, SDL_Color{ 150, 220, 140, 255 });
    }
}

void Game::renderTitleArt() {
    // This part of the code is for the wagon drives across a sky that cycles through the day
    const Uint64 now = SDL_GetTicks();
    const TimePart part = static_cast<TimePart>((now / 4000) % kPartsPerDay);
    const float travel = static_cast<float>(now % 14000) / 14000.0f;
    m_scene.drawScene(m_renderer, kTitleArea, part, now, 0.0f, travel);

    const std::string title = "THE SCARYGON TRAIL";
    const std::string subtitle = "A journey west. What could go wrong?";
    const float titleX = kTitleArea.x + (kTitleArea.w - m_text.textWidth(title, 4.0f)) * 0.5f;
    const float subX   = kTitleArea.x + (kTitleArea.w - m_text.textWidth(subtitle, 2.0f)) * 0.5f;

    SDL_SetRenderDrawColor(m_renderer, 40, 25, 20, 255);          
    m_text.drawLine(m_renderer, title, titleX + 3.0f, 43.0f, 4.0f);
    m_text.drawLine(m_renderer, subtitle, subX + 2.0f, 92.0f, 2.0f);
    SDL_SetRenderDrawColor(m_renderer, 250, 236, 200, 255);       
    m_text.drawLine(m_renderer, title, titleX, 40.0f, 4.0f);
    m_text.drawLine(m_renderer, subtitle, subX, 90.0f, 2.0f);
}

void Game::renderTop() {
    SDL_FRect topRect;
    topRect.x = 0;
    topRect.y = 0;
    topRect.w = static_cast<float>(m_windowWidth);
    topRect.h = static_cast<float>(m_windowHeight) * kTopPanelRatio;

    SDL_SetRenderDrawColor(m_renderer, 210, 190, 150, 255); 
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

    SDL_SetRenderDrawColor(m_renderer, 245, 240, 225, 255); 
    SDL_RenderFillRect(m_renderer, &bottomRect);

    // This part of the code makes a thin divider between the two panels
    SDL_FRect divider{ 0.0f, topHeight, static_cast<float>(m_windowWidth), 3.0f };
    SDL_SetRenderDrawColor(m_renderer, 120, 100, 70, 255);
    SDL_RenderFillRect(m_renderer, &divider);

    //This part of the code is for the narrative text, limited to the space above the buttons
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
    if (m_phase == Phase::NarratorShift) drawNarratorBanner();
    if (m_phase == Phase::Ending) drawEndingPlaque();
    renderBottom();

    SDL_RenderPresent(m_renderer);
}

void Game::shutdown() {
    if (m_renderer) saveProgress();   
    m_log.stop();
    if (m_renderer) {
        m_scene.releaseResources();
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}
