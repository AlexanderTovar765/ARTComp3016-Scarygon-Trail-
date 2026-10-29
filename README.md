# The Scarygon Trail

An amusing survival horror game that has been influenced by William’s *Oregon Trail*; developed in C++ utilizing the SDL3 system for COMP3016 (Immersive Game Technologies) coursework 1.

At every point along the route, there are brief narrative events in a monotonous delivery, regardless of the event, whether it involves a broken wagon wheel or something of that sort. It challenges players to keep a family of four alive by managing their food, health, sanity, and wagon condition, and to experience the consequences of their actions.

**Repository:** https://github.com/AlexanderTovar765/ARTComp3016-Scarygon-Trail-
**Video (unlisted YouTube, max 10 minutes):** PLACEHOLDER: add the YouTube link

## Contents
1. [How to run](#1-how-to-run)
2. [Proposal and deviations](#2-proposal-and-deviations)
3. [Gameplay description](#3-gameplay-description)
4. [Sample screens](#4-sample-screens)
5. [Dependencies](#5-dependencies)
6. [Use of AI](#6-use-of-ai)
7. [AI Development Log](#7-ai-development-log)
8. [Game programming patterns](#8-game-programming-patterns)
9. [Game mechanics and how they are coded](#9-game-mechanics-and-how-they-are-coded)
10. [UML design diagram](#10-uml-design-diagram)
11. [Exception handling and test cases](#11-exception-handling-and-test-cases)
12. [Playtesting evidence and reflection](#12-playtesting-evidence-and-reflection)
13. [Further details](#13-further-details)
14. [Evaluation](#14-evaluation)

## 1. How to run

**From the submitted zip (no Visual Studio needed, Windows x64):** unzip, open the folder, and run `ScarygonTrail.exe` from inside it. The executable needs `SDL3.dll`, the irrKlang DLLs, the `data/` folder and the `assets/audio/` folder beside it, and the zip includes all of them.

**From source:** requires Visual Studio 2022 with the "Desktop development with C++" workload. SDL3 (3.4.18, x64) is bundled in `vendored/SDL3`, so no separate install is needed.

1. Open `ScarygonTrail.sln`.
2. Select `Debug | x64` (or `Release | x64`).
3. Press F5 to build and run. `SDL3.dll`, `data/` and `assets/audio/` are copied next to the executable automatically; output goes to `build/`.

**Sound (optional).** irrKlang's licence allows its DLL to be redistributed with a non-commercial program but not the whole SDK, so the SDK is not in the repository. To build with sound, run once from the repository root:

```
powershell -ExecutionPolicy Bypass -File tools\get-irrklang.ps1
```

then rebuild with **Build > Rebuild Solution**. The project detects `vendored/irrKlang` and links it. Without it the title screen says "Sound: unavailable" and the game is silent.

**Controls**
- Mouse click or `1`-`4`: pick a choice
- `M`: mute or unmute sound
- `L` (title screen): turn the anonymous playtest log on or off
- `Esc`: quit

## 2. Proposal and deviations

Week 2 proposal: https://docs.google.com/document/d/1e3urBafL8XUcumxg3YoX4mpzwOVRAyJINFEv5XdRE8o/edit?tab=t.0#heading=h.hbigc4tx4p9x

### Deviations from the proposal

| Proposal said | What was built | Justification |
|---|---|---|
| A long trail with `TrailNode` objects | 3 days x 5 parts; `Trail` stores events by (day, part) | CW1 is 30% of the module, so the content was scaled. A map keyed by slot was simpler than linked nodes |
| Party members that can be sick, cursed, lost or dead | Healthy, Ill or Dead | "Cursed" was cut for scope. Individuality comes from frailty and choices that target one member |
| Scrolling text in the bottom panel | Wrapped text that fits the panel, checked on load | Scrolling added little for short events, and fit is tested |
| irrKlang to be requested if pursued | irrKlang used for sound | irrKlang is listed as permitted in the brief; it is optional at build time |
| The top panel would show "the wagon, the characters sprite, icons of inventory" | Everything is drawn in code from rectangles, circles and lines, with no sprites or image files. There is no inventory: the four stats (Food, Health, Sanity, Wagon) are the resources | No image assets to source, cite or approve, and the scaled-down scope had no room for an inventory system. The look was made deliberately retro after playtesting (see below) |
| "2-4 buttons" per event | Always exactly four choices | A fixed layout is simpler to build and test, and every event gives a real risk/reward spread |
| A wagon "in motion on the map" | The wagon is parked on the road during events (drawing across the screen on the title screen only), and a progress strip of 15 dots shows how far through the journey the player is | A map needs far more art and data than the scope allowed, and a progress strip tells the player where they are at a glance |
| Unreliable narration only (the less sane you are, the less reliable the game) | Same mechanic, plus a visible NARRATOR meter, a "narrator is slipping" screen, an end-of-day truth check and a lie count at the ending | **Playtest round 1:** both testers took the lying for ordinary bugs. The lying stays presentation only, so the game never cheats the player |
| No difficulty settings | Easy, Normal and Hard (`src/Difficulty.*`), changing only numbers (food, daily toll, harm from choices) | **Playtest round 2:** every tester rated difficulty 1 (too easy). Levels were tuned with the balance simulator (see `docs/balance-report.md`) |
| One fixed oxen picture for the whole game | A scene for each event (15) and a reaction animation for each choice (20), drawn in code, rendered in a chunky pixel-art style | **Playtest round 2:** testers saw no reason to play again. The scene picture is rendered small and scaled up for a 2D pixel look the player asked for |
| No save or progress system | The game remembers animations seen (of 35), endings found (of 7) and which difficulties were cleared in `save/progress.txt` | Gives a reason to replay without changing the rules of any single run. It is optional: a missing or unwritable file is harmless |
| Roadmap: one playtest round in Week 5 | Two rounds (late Oct), each followed by a change driven by the feedback | One round did not show whether the first fix worked, so a second was added (see section 12) |
| Sound was an "if looking later" idea | Ambient music that follows the narrator's state, plus effects, synthesised by `tools/make_sounds.py` | Avoids sourcing or licensing audio. Optional at build time (the game is silent without irrKlang) |

## 3. Gameplay description

Three playable days. Each day has 5 parts (Dawn, Mid-Morning, Noon, Evening, Night), one event per part, and four choices per event. There are 20 events in total: 15 normal events plus 5 "desperate" variants that replace a normal event when the party is in trouble. Events are loaded from `data/events.txt`.

- **A family of four** (Ma, Pa, Junior, Granny) with portraits in the top panel. Each has their own health and can fall ill or die, and some choices hit one person only. The Health bar is the family average, and the dead count as 0.
- **Resources:** Food, Health, Sanity and Wagon, shown as bars. The trail takes a toll after every part.
- **Difficulty:** Easy, Normal or Hard, chosen on the title screen (and again on the ending screen) and shown as a tag beside the narrator meter. The levels change only numbers: starting food, how fast the trail wears the family down, and how hard bad choices hit. The story is identical on all three.
- **Sanity changes how trustworthy the game is:** Stable above 60, Unreliable from 30 to 59 (some words and HUD numbers are wrong), Breakdown below 30 (numbers hidden, lies freely). Only the presentation lies. What choices do, and the "what happened" summary, are always true.
- **The lying is made visible:** a NARRATOR meter in the HUD (TRUSTWORTHY / SHAKY / LYING), a screen when the narrator slips, breaks down or recovers, an end-of-day truth check naming something it got wrong, and a lie count at the ending.
- **Desperate events:** when food, the wagon or health drops into danger, a warning appears and a harsher version of a later event can replace the normal one.
- **Animated scenes:** every event has its own scene in the top panel (a map seller, a deer, a river crossing, a ritual circle...) and every choice plays its own reaction when chosen, drawn in code in a pixel-art style. There are 15 scenes and 20 reactions.
- **Discoveries:** the title screen counts the animations seen (of 35) and endings found (of 7), and notes which difficulties have been cleared. This is saved between runs, so each new journey can show something not seen before.
- **Screens:** title, how to play, events, results, day summaries and an ending summary showing who survived and how far the family got.
- **Sound:** ambience that follows the narrator state, plus effects for choices, warnings and deaths. Optional at build time.
- **Ending:** the run ends if the whole family dies, or Sanity or the Wagon reaches 0, or the three days are completed. There are seven different endings, depending on who survived and how the journey went.

Playthroughs differ because the choices lead to different stats, different desperate events, different narrator states and different endings, and each run can show new animations the player has not seen yet. Together with the three difficulty levels, that is the reason to play a second time.

## 4. Sample screens

| Title | How to play |
|---|---|
| (<img width="952" height="629" alt="Screenshot 2026-10-29 095827" src="https://github.com/user-attachments/assets/fbb68e28-b91a-43db-ab7a-10cd31ad0d7c" />) |(<img width="949" height="631" alt="Screenshot 2026-10-29 095900" src="https://github.com/user-attachments/assets/f99f39f9-e346-457a-802f-dc5c8ca2e7bd" />) |

| An event | A result with a warning |
|---|---|
| (<img width="949" height="625" alt="Screenshot 2026-10-29 095939" src="https://github.com/user-attachments/assets/f8a112e0-6d29-4590-b23c-ac166eb7123a" />) |(<img width="944" height="626" alt="Screenshot 2026-10-29 100032" src="https://github.com/user-attachments/assets/12ed57da-6c14-4b4f-9176-bdd7c6dfc082" />) |

| Ending summary |
|---|
| (<img width="938" height="619" alt="Screenshot 2026-10-29 100107" src="https://github.com/user-attachments/assets/71fab3ca-994a-4e38-8563-114864e7996c" />) |


## 5. Dependencies

- **SDL3 3.4.18** (bundled in `vendored/SDL3`): window, rendering, input and the built-in debug font.
- **irrKlang 1.6.0** (optional, downloaded by `tools/get-irrklang.ps1`): sound.
- Nothing else beyond the standard C++17 library.

## 6. Use of AI/AI Development Log

I have used ChatGPT as I mentioned in my AI log. Although it was mentioned in my project proposal, I have noted how I have used it in my work on this project. For instance, AI helped in writing various parts of my C++ code, documentation, the text of the event, various types of tests, balance simulator, and sound script. I took important decisions about design such as humorous horror tone, split-panel layout, three-day framework, and that unreliable narration impacts just the presentational aspect but does not affect gameplay. For example, I had balance simulator showing 99.9% survival with random gameplay, so I had the idea to add penalties for tolls and starvation. One of the tests also predicted that the wrong health is going to be decreased because of incorrect arithmetic, so I had to fix the test rather than the game code. Lastly, I have made the unreliable narration that shifts at least one word.
My contributions can be tracked in commits like 587643e, bbf2a29, and 1b3f873 where I worked on the event loader, Day 1 events, Days 2 and 3 with the narrator and the balance simulator. I have checked the work done by AI by compiling

The full account is in the AI Development Log (section 7). The signed Generative AI Declaration is included as an appendix: https://liveplymouthac-my.sharepoint.com/:w:/r/personal/alexander_tovar_students_plymouth_ac_uk/Documents/Generative%20AI%20Tool%20declaration%20template%20-%20All%20-%20v1.0.docx?d=wd999a88252224706b1061d8d156a7792&csf=1&web=1&e=hTR9pb

See [docs/ai-dev-log.md](docs/ai-dev-log.md) for the AI tools used, categories of use, example prompts, the verification process, and repository evidence.

## 7. Game programming patterns

| Pattern | Where | Why |
|---|---|---|
| Game Loop | `Game::run`: handle input, update, render each frame | Standard structure for a real-time window |
| State | `NarrativeState` with `StableNarration`, `UnreliableNarration` and `BreakdownNarration` (in `NarrativeState.cpp`) | Sanity changes how trustworthy the presentation is. `Game` swaps one object and calls the same functions |
| Observer | `PartyObserver`, `ThresholdWatcher`, `MemberWatcher` | `Party` announces changes without knowing who is listening, so warnings and illness messages are decoupled from the rules |
| Composition | `Party` owns four `PartyMember` objects | The party is made of its members, and health is derived from them |
| Data-driven content | `Trail::loadFromFile` reads `data/events.txt` | Events can be written and edited without recompiling |
| Pimpl idiom | `AudioManager::Impl` | Keeps irrKlang out of the header. The same class is a silent stand-in when irrKlang is not built in |
| Parameter object (rules table) | `DifficultyRules` and `rulesFor(Difficulty)` in `Difficulty.cpp`, used by `Party::rules()` and `Party::adjusted()` | Easy, Normal and Hard are one struct of numbers. `Party` never branches on the difficulty name, so adding a fourth level means adding one row |
| Value-to-name mapping | `SceneId` / `ActionId` enums with `sceneName` / `parseScene` and `actionName` / `parseAction` in `Animations.cpp` | The data file uses readable names, the code uses enums, and tests check every name round-trips |
| Dispatch on an enum | `stage::drawSceneActor` and `stage::drawAction` in `SceneActors.cpp` | Each scene and reaction is one small drawing function chosen by its ID, so adding an animation does not touch the others |
| Persistence with defensive loading | `Progress::load` / `save` (`save/progress.txt`) | Progress survives restarts, and a missing, damaged or read-only file is ignored, in keeping with the "no corrupted file can crash it" requirement |
| Off-screen render and scale (pixel art) | `SceneRenderer::beginPixelPass` / `endPixelPass` | The scene is drawn into a small texture and scaled up with no smoothing, so every shape becomes chunky pixels while text is drawn afterwards at full sharpness |

The newer features follow the same idea: difficulty is a table of numbers `Party` reads, and each event's scene and each choice's reaction animation are chosen by name from `events.txt`, so balance and content can change without rewriting the game's logic.

## 8. Game mechanics and how they are coded

| Mechanic | How it works | Code |
|---|---|---|
| Day and part progression | `Trail` tracks the current day and part; 15 parts in total, then the trail ends | `Trail::advance`, `Trail::getEvent` |
| Choices and effects | Each choice carries `Effects` (food, health, sanity, wagon and an optional member). `Party::adjusted` scales them for the chosen difficulty, then `Party::apply` applies them, clamped to 0-100 | `Effects`, `Party::adjusted`, `Party::apply` |
| The trail's toll | After every part the party eats, sanity and the wagon wear, every living member loses health (scaled by frailty), and starvation hurts at 0 food. The amounts come from the difficulty's rules | `Party::advanceTime`, `Party::rules` |
| Individual members | Health out of 100 and a frailty percentage; Ill below 40, Dead at 0; each death costs the survivors sanity | `PartyMember`, `Party::applyGrief` |
| Targeted choices | A choice with `member = Pa` hits only that member | `Effects::member` |
| Desperate events | An event with `requires = LowFood` replaces the normal event while that condition is active | `ThresholdWatcher`, `Trail::getEvent` |
| Unreliable narration | Text swaps and HUD jitter depend on a per-screen seed, so nothing flickers. The "what happened" line is always true. A NARRATOR meter, a shift banner and an end-of-day truth check show the player when it is lying | `NarrativeState`, `LieTracker`, `Game::onEventChoice` |
| Difficulty | Easy, Normal and Hard are three rows of numbers (starting food, daily toll, and how much damage and help choices do). Changing the level only changes which row `Party` reads, so the story and events never change | `Difficulty.cpp` (`rulesFor`), `Game::cycleDifficulty` |
| Scenes and reactions | Each event names a scene and each choice names a reaction in `events.txt`. The scene is drawn while the event is on screen, and the reaction plays on the result screen. The picture is drawn small and scaled up for the pixel-art look | `Animations.cpp`, `SceneActors.cpp`, `SceneRenderer::drawStage` |
| Discoveries and replay | The game records which scenes, reactions and endings have been seen and which difficulties have been cleared, and shows the totals on the title screen. Saved between runs and ignored safely if the file is missing or damaged | `Progress` (`markScene`, `markAction`, `markEnding`, `classify`), `save/progress.txt` |
| Endings | The finish is classified into one of seven endings from the survivors and the final sanity and wagon, and the ending screen says whether it is new | `Progress::classify`, `Game` ending screen |
| Sound | Ambience follows the narrator state; effects play for choices, warnings and deaths | `AudioManager` |
| Playtest log | Optional anonymous CSV of the choices made | `PlaytestLog`, `Settings` |

## 9. UML design diagram
New Design:
```mermaid
classDiagram
    class Game { +init() +run() }
    class Party { +apply(Effects) +advanceTime() +isAlive() +addObserver() }
    class PartyMember { +changeHealth(int) +getStatus() }
    class PartyObserver { <<interface>> +onStatChanged() +onMemberStatusChanged() }
    class ThresholdWatcher { +active() +takeWarnings() }
    class MemberWatcher { +takeMessages() }
    class Trail { +loadFromFile() +getEvent() +advance() }
    class Event { +getText() +getChoices() +getCondition() }
    class Choice { +getLabel() +getEffects() }
    class Effects
    class NarrativeState { <<abstract>> +filterText() +shownValue() }
    class StableNarration
    class UnreliableNarration
    class BreakdownNarration
    class AudioManager
    class PlaytestLog
    class Settings
    class SceneRenderer
    class TextRenderer
    class Button
    Game *-- Party
    Game *-- Trail
    Game *-- ThresholdWatcher
    Game *-- MemberWatcher
    Game *-- AudioManager
    Game *-- PlaytestLog
    Game o-- NarrativeState
    Game *-- SceneRenderer
    Game *-- TextRenderer
    Game *-- Button
    Party *-- "4" PartyMember
    Party o-- PartyObserver
    PartyObserver <|-- ThresholdWatcher
    PartyObserver <|-- MemberWatcher
    Trail *-- Event
    Event *-- Choice
    Choice *-- Effects
    NarrativeState <|-- StableNarration
    NarrativeState <|-- UnreliableNarration
    NarrativeState <|-- BreakdownNarration
```

Old Design: <img width="446" height="587" alt="Screenshot 2026-10-22 093603" src="https://github.com/user-attachments/assets/8f8d3295-d754-4473-a2c7-cbcd432fc2f6" />
  
## 10. Exception handling and test cases

The code does not use C++ exceptions. Instead it validates input and reports problems, so bad data cannot crash the game:

- **Event file:** a missing or empty file shows a clear message. A malformed event is skipped with a warning that names the line. Text and numbers are cleaned (byte-order marks, Windows line endings and curly quotes are handled, and out-of-range effects are clamped). An unknown `scene` or `action` name only produces a warning, and the event still loads.
- **Settings file:** a missing file is fine. Bad lines are reported and defaults are used.
- **Progress file:** a missing, damaged or read-only `save/progress.txt` is ignored. The game starts with no discoveries on Normal difficulty and never crashes.
- **Audio:** missing irrKlang, no sound device, or a missing sound file never stops the game.
- **Playtest log:** if the log file cannot be created, logging is switched off quietly.
- **Rendering:** if the off-screen texture for the pixel-art picture cannot be created, the scene is drawn normally instead.
- **Input:** keys that match no button are ignored.

**Tests:** `tests/logic_tests.cpp` has 35 test groups (22,238 checks), all passing, with the silent audio stub and with the real irrKlang engine. They now also cover the lie tracker, difficulty rules and scaling, animation names, the shipped events' scenes and actions, and the progress file. Details are in [docs/test-cases.md](docs/test-cases.md). The balance simulation (20,000 games per strategy on each of the three difficulties) is in [docs/balance-report.md](docs/balance-report.md).

To run the tests (from the repository root, because they read `data/` and `assets/audio/`):

    cmake --build build --target logic_tests
    ctest --test-dir build --output-on-failure
    
## 11. Playtesting evidence and reflection

Evidence (session log, observation notes, questionnaire results) is in [docs/playtesting.md](docs/playtesting.md). Two structured rounds were run, and each round's feedback led to a code change linked to a commit (table below). 

### Playtesting reflection

- **Who participated:** 4 different people (A, B, C and D, anonymised: friends and one classmate) over 5 sessions. Tester E was the same person as Tester A, returning to check whether the round 1 fix worked. Round 1 was on 23 October (2 testers) and round 2 on 28-29 October (3 sessions).
- **What was tested:** one complete playthrough each at their own pace, with no help. I filled in an observation sheet while they played, then they completed the questionnaire alone, and I asked for their best moment and the first thing they would change. Round 1 tested the original build (`3038a1c`). Round 2 tested the build with the visible narrator changes (`b4a77f3`).
- **Feedback received:**
  - Everyone understood the game without help (5/5 from all five sessions) and liked its quirky style (fun averaged 3.8/5).
  - **Round 1:** both testers scored 2/5 on "I noticed when the game started lying to me", so they took the typos and wrong numbers for ordinary behaviour.
  - **Round 2:** noticing the lying rose to 4, 5 and 5 (average 4.7) after the narrator was made visible. One tester noticed the mechanic without being told.
  - **Difficulty:** all five sessions rated it 1, "too easy", including the one tester who did not reach the end.
  - **Replay:** "I would play it again" averaged 3.2/5, and testers said there was little reason to play a second time. The jokes divided people (averaging 2.8/5).
- **Changes implemented:**

| Feedback | Change | Commit |
|---|---|---|
| Round 1: the lying was not noticed | A NARRATOR meter, a "narrator is slipping" screen, an end-of-day truth check naming something it got wrong, and a lie count at the ending | `b4a77f3` |
| Round 2: too easy (all rated 1) | Easy / Normal / Hard difficulty (Random play now survives about 89% / 48% / 6%) | `a8f589a` |
| Round 2: no reason to replay | A new scene for every event and a reaction for every choice, a count of animations and endings discovered, and a pixel-art look | `a8f589a` |

- **Remaining limitations:**
  - Only 4 different people tested the game, all friends or classmates, so the results show trends and not statistics.
  - **The difficulty levels, animations and discoveries have not been playtested.** Round 2 ran on the build before them, so I cannot yet show that they fixed "too easy" or "no reason to replay". The balance simulator is my evidence for difficulty until a third round.
  - The jokes divided testers, and the humour was not changed.
  - With more time I would add more days and another character and stat, and improve the graphics further, though this was my first use of SDL3.

## 12. Further details-COME BACK TO

- **Project structure**

```
src/      C++ source (Game, Party, PartyMember, observers, Trail, NarrativeState, LieTracker, Difficulty, Animations, SceneActors, Progress, renderers, audio, settings, playtest log)
tests/    logic_tests.cpp, automated tests
tools/    balance_sim.cpp (plays thousands of games on each difficulty to check balance), make_sounds.py (synthesises the sounds), get-irrklang.ps1
vendored/ bundled SDL3 headers and x64 libraries (irrKlang is downloaded here by the script, not committed)
assets/   audio/ (generated WAV files)
data/     events.txt (loaded at runtime, including each event's scene and each choice's action; format explained at the top of the file) and settings.txt
docs/     proposal, AI Development Log, balance report, test cases, playtesting
save/     progress.txt, written by the game at runtime (not committed)
```

- **Anonymous playtest log:** off by default. It can be turned on in `data/settings.txt` or with `L` on the title screen. It records the choices made, the result and the difficulty, with no names or personal data.
- **Remembered progress:** the game writes `save/progress.txt` (chosen difficulty, number of runs, which animations and endings have been seen, which difficulties have been cleared). It contains no personal data, is optional, and can be deleted at any time to reset discoveries.
- **Git evidence:** PLACEHOLDER: add a screenshot of your GitHub Insights contribution graph and of key milestone commits.

## 13. Evaluation

**What I achieved.** I built The Scarygon Trail, a complete comedic-horror survival game in C++ and SDL3: three days, 20 events with four choices each, a family of four with individual health, a narrator that becomes less trustworthy as sanity falls, three difficulty levels, an animated scene for every event and a reaction for every choice, and seven endings. It is data-driven (events live in `data/events.txt`), it validates its files instead of crashing, and it has 35 automated test groups (22,238 checks) plus a balance simulator. The State and Observer patterns from my proposal are both used for real jobs, not added for show. Most of the proposal was delivered, and the places it changed are recorded in section 2.

**What playtesting taught me.** My own testing told me the game worked. Players told me whether it was good. In round 1 neither tester noticed the narrator lying, because I had made the best mechanic in the game too subtle to see, so I added the narrator meter, the shift screen and the end-of-day truth check. By round 2 the testers who played noticed it, and one noticed it without being told. Round 2 then showed a problem I could not see myself: every tester rated the difficulty 1 and there was no reason to replay. I had balanced the game with a simulator that played well-defined strategies, and I had trusted its numbers too much. Real players are not the strategies I wrote, so I added difficulty levels and rebuilt the pictures around the player's choices. I was surprised at the efficiency new players were at completing the game without any help, a testament to how air tight I tried to make the game.

**What I would do differently.** I would playtest much earlier. Both of my biggest changes came from feedback that I could have had a week sooner with a rough build. I would also write the Observer and Difficulty ideas into the proposal with firm numbers, so the first balance pass was not guesswork. I would test the final build with a third group of players, because the difficulty levels and animations were added after the last round and have not been playtested.

**What I am proudest of.** I am most proud of the final game. Having it as an exe file will mean I can hold onto it for years afterward and I'm excited to show and send it to family and friends as a working program that I created.

**With more time.** More days and events, another stat and character so the choices have more to trade off, better art and animation, and a larger and more varied set of playtesters. I would also add settings for sound and difficulty in-game, and an event editor to speed up writing content.

Overall, having AI assist in the full process of the game was a bizarre idea at first, but I see now its true intention is to make as large a game as possible in a short time. I am content with my usage of AI but am more elated by the final product.
