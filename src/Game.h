#pragma once
#include <SDL3/SDL.h>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "Button.h"
#include "NarrativeState.h"
#include "Party.h"
#include "SceneRenderer.h"
#include "TextRenderer.h"
#include "Trail.h"

class Game {
public:
    Game();
    ~Game();

    bool init(const char* title, int width, int height);
    void run();

private:
    // Which screen the bottom panel is showing. Button presses mean different things in each.
    enum class Phase {
        Event,       // an event with up to 4 choices
        Result,      // what your choice did, with a Continue button
        DaySummary,  // end-of-day recap
        Ending       // death, end of content, or a load error
    };

    void handleInput();
    void update();
    void renderTop();
    void renderBottom();
    void render();
    void shutdown();

    // Game flow
    void startGame();
    void showCurrentEvent();
    void showDaySummary();
    void showEnding(const std::string& text, bool canRestart);
    void onEventChoice(std::size_t index);
    void onResultContinue();
    void onDaySummaryContinue();
    void selectChoice(std::size_t index);

    // Sanity -> how trustworthy the game is (State pattern, see NarrativeState.h)
    void setNarrator(NarrativeKind kind);
    void updateNarrator();

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

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    bool m_running = false;

    int m_windowWidth  = 0;
    int m_windowHeight = 0;

    TextRenderer        m_text;
    SceneRenderer       m_scene;
    Party               m_party;
    Trail               m_trail;
    Phase               m_phase = Phase::Event;

    std::unique_ptr<NarrativeState> m_narrator;    // current state: Stable / Unreliable / Breakdown
    unsigned            m_screenSeed = 0;          // changes per screen, constant within one
    std::string         m_pendingNotice;           // one-off line when the narrator state changes

    std::string         m_narrative;               // text shown in the bottom panel
    std::vector<Button> m_buttons;
    float               m_buttonsTop = 0.0f;       // y of the first button row, set by layoutButtons()
    bool                m_canRestart = false;      // does the Ending screen offer "Play again"?

    // Split ratio: top panel takes this fraction of the window height.
    // Matches the proposal's ~60% top / ~40% bottom layout.
    static constexpr float kTopPanelRatio = 0.6f;
};
