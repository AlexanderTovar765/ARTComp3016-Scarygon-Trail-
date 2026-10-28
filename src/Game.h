#pragma once
#include <SDL3/SDL.h>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AudioManager.h"
#include "Button.h"
#include "LieTracker.h"
#include "MemberWatcher.h"
#include "NarrativeState.h"
#include "Party.h"
#include "PlaytestLog.h"
#include "Progress.h"
#include "SceneRenderer.h"
#include "Settings.h"
#include "TextRenderer.h"
#include "ThresholdWatcher.h"
#include "Trail.h"

class Game {
public:
    Game();
    ~Game();

    bool init(const char* title, int width, int height);
    void run();

private:
    // Which screen is showing. Button presses mean different things in each.
    enum class Phase {
        Title,        // title screen: start, how to play, quit
        Instructions, // how to play
        Event,        // an event with up to 4 choices
        Result,       // what your choice did, with a Continue button
        NarratorShift,// the narrator's state just changed: a screen that says so
        DaySummary,   // end-of-day recap
        Ending        // death, end of content, or a load error
    };

    void handleInput();
    void update();
    void renderTop();
    void renderHud();
    void renderTitleArt();
    void drawNarratorMeter();
    void drawNarratorBanner();
    void drawEndingPlaque();
    void drawDifficultyTag(float x);
    void renderBottom();
    void render();
    void shutdown();

    // Game flow
    void showTitle();
    void cycleDifficulty();
    void saveProgress();
    void showInstructions();
    void startGame();
    void showCurrentEvent();
    void showDaySummary();
    void showEnding(const std::string& text, bool canRestart, bool withSummary = true);
    void onEventChoice(std::size_t index);
    void onResultContinue();
    void proceedAfterResult();
    void showNarratorShift();
    void onNarratorShiftContinue();
    void onDaySummaryContinue();
    void selectChoice(std::size_t index);
    std::string journeySummary() const;

    // Sanity -> how trustworthy the game is (State pattern, see NarrativeState.h)
    void setNarrator(NarrativeKind kind);
    static Mood moodFor(NarrativeKind kind);

    // Bottom panel
    // 'narrative' is run through the current NarrativeState (it may lie) when 'lie' is true.
    // 'factual' is appended afterwards and is never altered, so results stay trustworthy.
    void showScreen(const std::string& narrative, const std::vector<std::string>& buttonLabels,
                    bool lie = false, const std::string& factual = "");
    void layoutButtons();
    void handleMouseMotion(float x, float y);
    void handleMouseClick(float x, float y);

    // Top panel
    void drawStatBar(const char* label, Stat stat, int actualValue, float y, SDL_Color fill);
    void drawMemberCard(std::size_t index, float x);

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    bool m_running = false;

    int m_windowWidth  = 0;
    int m_windowHeight = 0;

    TextRenderer        m_text;
    SceneRenderer       m_scene;
    Settings            m_settings;
    AudioManager        m_audio;
    PlaytestLog         m_log;

    Party               m_party;
    ThresholdWatcher    m_thresholds;      // observer: low-stat warnings and desperate events
    MemberWatcher       m_memberWatcher;   // observer: illness and death messages
    Trail               m_trail;
    const Event*        m_activeEvent = nullptr;  // the event on screen (may be a desperate variant)
    int                 m_partsPlayed = 0;
    Phase               m_phase = Phase::Title;

    std::unique_ptr<NarrativeState> m_narrator;    // current state: Stable / Unreliable / Breakdown
    unsigned            m_screenSeed = 0;          // changes per screen, constant within one
    std::optional<NarrativeKind> m_shiftTo;        // set when sanity crossed a threshold: the narrator changes after this result
    Progress            m_progress;                // endings, scenes and animations discovered; last difficulty
    SceneRenderer::Stage m_stage;                  // which scene and reaction the top picture shows
    Uint64              m_stageStartMs = 0;        // when the result (and its reaction animation) appeared
    int                 m_newDiscoveries = 0;      // new scenes/animations found this journey
    std::optional<EndingId> m_ending;              // how this journey ended (set on the Ending screen)
    bool                m_endingIsNew = false;
    LieTracker          m_lies;                    // how often the narrator misled the player (revealed at the end of each day)

    std::string         m_narrative;               // text shown in the bottom panel
    std::vector<Button> m_buttons;
    float               m_buttonsTop = 0.0f;       // y of the first button row, set by layoutButtons()
    bool                m_canRestart = false;      // does the Ending screen offer "Play again"?

    // Split ratio: top panel takes this fraction of the window height.
    // Matches the proposal's ~60% top / ~40% bottom layout.
    static constexpr float kTopPanelRatio = 0.6f;
};
