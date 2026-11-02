# AI Development Log: The Scarygon Trail

This element is **Partnered Work**: AI was an integral part of development, and this log is meant to make that use transparent. AI wrote most of the code, art, sound script and first-draft documentation. My role was the concept, design decisions, directing and correcting the AI, building and playing every version in Visual Studio 2022, running the playtests, and deciding what to change from the feedback.

## 1. AI tools used

| Tool | Used for |
|---|---|
| Claude (Anthropic), via claude.ai | Most of the C++ code, all code-drawn art and animation, UI screens, the sound synthesis script, event text drafts, balancing, refactoring, automated tests, debugging, and drafting the README and docs |
| ChatGPT (OpenAI), via ChatGPT.com | [CHECK: what you really used it for, e.g. early concept and proposal drafting] |

No image, music or audio generation models were used. All artwork is C++ drawing code (rectangles, circles, lines) written by AI, and all sound comes from a Python synthesis script written by AI that I ran. The only third-party library besides SDL3 is irrKlang, which the brief lists as permitted.

## 2. Categories of use

Each row is one of the uses the brief lists, plus the "code assistant", "programming testing" and "readme report crafting" uses allowed at the Partnered level.

| Category | What AI produced | What I did | Where |
|---|---|---|---|
| Sprite generation and character artwork | Code-drawn family portraits (Ma, Pa, Junior, Granny), the wagon and oxen, and a scene for each of the 15 events. There are no image files: every "sprite" is drawn from shapes in C++ | Described what each scene and character should show, checked them in screenshots and in game, and asked for fixes (e.g. sun and hills drawn outside the frame, fixed with a clip rectangle) | `src/SceneRenderer.*`, `src/SceneActors.*` |
| Sprite animation | 20 choice reactions, the title-screen wagon, and the pixel-art pass (scene drawn to a small texture and scaled up) | Asked for animations after round 2 testers saw no reason to replay, chose the pixel-art look, and reported shapes vanishing at low resolution (now drawn from square blocks) | `src/SceneActors.*`, `src/Animations.*`, `SceneRenderer::beginPixelPass` |
| UI assets | Buttons, stat bars, NARRATOR meter, title, how-to-play, result, day summary, narrator-shift and ending screens | Designed the split-panel layout (graphics on top, text and choices below), asked for the title and instructions screens, and asked for the meter after round 1 testers missed the lying | `src/Game.*`, `src/Button.*`, `src/TextRenderer.*` |
| Narration and dialogue | Drafts of event text, choice labels, result lines, warnings, the narrator's lies and the end-of-day truth checks | Set the concept and the deadpan comedic-horror tone, wrote example prompts the events had to match, and edited the drafts. [CHECK: name any events you wrote or rewrote yourself] | `data/events.txt`, `src/NarrativeState.*`, `src/LieTracker.*` |
| Sound effects | `make_sounds.py`, which synthesises the ambience and effects, and the code that changes ambience with the narrator state | Ran the script and the irrKlang download, rebuilt, and checked the sound on my machine | `tools/make_sounds.py`, `src/AudioManager.*` |
| Balancing suggestions | A balance simulator (20,000 games per strategy), tolls, thresholds and the Easy/Normal/Hard numbers | Decided the game was too easy when the first run showed 99.9% survival, and acted on playtesters rating difficulty 1 by asking for difficulty levels | `tools/balance_sim.cpp`, `src/Difficulty.*`, `docs/balance-report.md` |
| Optimisation suggestions | Drawing each scene once into a small texture and scaling it up; a per-screen seed so the lying text is worked out once per screen instead of flickering every frame; keeping irrKlang out of the header (Pimpl) | Checked the game ran smoothly in Visual Studio | `SceneRenderer`, `NarrativeState`, `AudioManager::Impl` |
| Refactoring assistance | Replacing the proposal's linked `TrailNode` list with events keyed by (day, part); splitting the party into four `PartyMember` objects with an Observer for warnings; turning difficulty into one table of numbers so `Party` never branches on it; adding CMake support | Chose to scale the game to 3 days x 5 parts, and kept State and Observer from my proposal | `src/Trail.*`, `src/Party*.*`, `src/Difficulty.*` |
| Code generation (code assistant) | Most of the source files | Directed each feature, built and ran every version, made my own edits to Button, Conditions, Effects and Event (19-22 Oct), and can explain the code (section 8) | `src/` |
| Testing (programming testing) | `tests/logic_tests.cpp` (22,238 checks), headless playthroughs and screenshot checks run in the AI's own environment | Played every build myself and ran two rounds of human playtesting | `tests/`, `docs/test-cases.md`, `docs/playtesting.md` |
| Debugging | Fixes for build errors, leftover merge-conflict markers and git push failures | Pasted build logs and errors, and confirmed the fixes worked in Visual Studio | |
| Documentation (readme report crafting) | Drafts of the proposal, README, test cases, balance report and playtesting forms | Edited them, filled in playtest results, and wrote the evaluation | `README.md`, `docs/` |

## 3. Representative prompts (my real prompts, sampled and condensed)

| Stage | Prompt | What came back |
|---|---|---|
| Concept | "I am building a game where the prompt 'Your son has not been fed for 5 days and has converted to satanism in an attempt to curse you' is not out of place." Then: "you've misunderstood, this is not the focal point prompt, its one of many." | Proposal wording and tone, reworked after I redirected it |
| Tone | "Comedic horror is the tone I am focusing on with this project." | A deadpan style for event text |
| Layout | "The top screen would have the 2D graphics showing inventory or characters or wagon, and the bottom would have the text and options for button pressing." | The split-panel skeleton |
| Scope | "I have 6 weeks to make the game, so this is how I'm scaling it. The game will consist of 3 days total with 5 parts of the day: dawn, mid day, noon, evening, night. One event per part." | Day/part system and the event file format |
| Review | "Is there any part the could be included or improved to create a more user friendly game?" | Including sound effects, opening page and instructions would greatly improve the users enjoyment of The Scarygon Trail |



## 4. Chronological log

### Week 2: proposal and skeleton
- **AI did:** drafted the proposal sections, suggested the layout of the UML sketch, and wrote the SDL3 split-panel skeleton.
- **I decided:** the concept, the comedic-horror tone, the two-panel layout, and what went into the proposal.
- **Verified:** I ran the skeleton in Visual Studio and saw the window and panels.
- **Issue and fix:** the first roadmap went beyond the 6 weeks available and had to be scaled back.

### Week 3: data-driven events and bottom panel
- **AI did:** wrote the `Effects`, `Party`, `TimeOfDay`, `Trail` and `Button` classes, the defensive `events.txt` loader, text wrapping and the Day 1 event drafts.
- **I did:** chose the 3 days x 5 parts x 4 choices structure, edited the Day 1 events, and built and played Day 1 in Visual Studio.
- **Verified:** the loader was tested on missing, empty and malformed files (now test groups T18a-c).
- **Issue and fix:** after a pause the project stopped building. I pasted the build log; the cause was merge-conflict markers left in `main.cpp`, `Game.h` and `Game.cpp` after a pull. Clean copies fixed it.
- **Issue and fix:** a zip with only changed files left me without the `.sln` and SDL3, so Visual Studio would not run it. Complete zips plus CMake support fixed it.

### Week 4: three days, State pattern, art, balance
- **AI did:** drafted Days 2-3 so they build on Day 1's events, wrote `NarrativeState` (Stable / Unreliable / Breakdown by sanity), the code-drawn wagon scene and the balance simulator.
- **I decided:** sanity only changes presentation, never what choices do, so the game never cheats the player.
- **Verified:** the simulator plays thousands of games with four strategies. The first results showed 99.9% survival for random play, so tolls were added.
- **Issue and fix:** the sun and hills spilled out of the picture frame; a clip rectangle fixed it (found from screenshots).
- **Issue and fix:** lying text sometimes did not change at all; it now always swaps at least one word.

### Week 5: Observer, party members, screens, sound, playtest log
- **AI did:** suggested what to add to make the game more user friendly, then wrote `PartyMember`, the Observer watchers, desperate events, the title and how-to-play screens, the portraits, the sound script and `AudioManager`, and the playtest log.
- **I did:** chose which suggestions to keep, pushed the work, ran the irrKlang download script, rebuilt, and confirmed sound on my machine.
- **Verified:** 25 automated test groups (14,202 checks) passed with the silent audio stub and with real irrKlang; the AI ran headless playthroughs to every ending and checked screenshots; the balance simulation was re-run (Random play 67.1% survival).
- **Issue and fix:** a test expected a full party's health to drop by 3 per part. My arithmetic, and the AI's, was wrong; the average drops by 4 because frailer members take more damage. The test was corrected, not the code.

### Week 6: playtest round 1 feedback, making the lying visible
- **AI did:** helped me work out why round 1 testers did not notice the narrator lying (both took the typos and wrong numbers for ordinary bugs), proposed ways to make it obvious without changing what choices do, and wrote the changes.
- **I did:** ran round 1 with 2 testers, recorded the results in `docs/playtesting.md`, chose which feedback to act on, and pushed the changes.
- **Change made (commit `b4a77f3`):** a coloured NARRATOR meter in the HUD (TRUSTWORTHY / SHAKY / LYING), a full-screen "the narrator is slipping / has broken down / has recovered" moment, an end-of-day truth check naming one thing the narrator got wrong, and a lie counter on the ending screen (`src/LieTracker.*`).
- **Verified:** 28 automated test groups passed (T24-T26 added for the lie tracker); the AI's headless playthroughs and screenshots checked the new screens fit; the balance was unchanged because the lying is presentation only.
- **Issue and fix:** the ending screen showed the meter as TRUSTWORTHY at 1 sanity. The display was corrected so the HUD, bars and member cards agree.
- **Outcome:** in round 2 one tester noticed the lying mechanic without being told.

### Week 7: playtest round 2 feedback, difficulty and animations
- **AI did:** turned two pieces of round 2 feedback into a design (all testers rated difficulty 1, and there was no reason to play again), proposed Easy / Normal / Hard and their numbers, and wrote the difficulty system, the 15 event scenes, 20 choice reactions, the discoveries count and the pixel-art rendering.
- **I did:** ran round 2 with 2 new testers plus one returning from round 1, chose the fixes and the pixel-art look, built and played the result, and pushed it (commit `a8f589a`).
- **Verified:** 33 test groups (22,238 checks) passed; the simulator was extended to all three difficulties (Random play survives 88.9% / 48.3% / 5.8%); the AI ran headless playthroughs to the ending on each difficulty and checked every scene and reaction in screenshots.
- **Issue and fix:** the first Easy and Hard settings were wrong (Easy let nearly everyone through, Hard was close to impossible even for careful play). A parameter sweep in the simulator found settings that fit the targets.
- **Issue and fix:** the first pixel-art version made thin shapes disappear (hills, people, wagon wheels). They are now drawn from square blocks.
- **Issue and fix:** the real irrKlang test build crashed about 1 run in 10 at exit on the AI's Linux test environment. valgrind showed a use-after-free inside irrKlang's own thread, not game code, so it is documented in `docs/test-cases.md` instead of changing the game.
- **I decided:** difficulty rules, story and the narrator's lying stay separate. Only numbers change between levels, so one set of events and tests covers all three.

## 5. Problems found in AI output (and what that taught me)

| Problem | How it was found | Fix |
|---|---|---|
| AI said a build had "no warnings" before it had finished compiling everything | It was corrected when the full compile ran | Now every claim comes with the compile or test output |
| An AI scripted edit silently did nothing because the text it expected was not in the file (Python `replace` does not fail on a miss) | A later compile error showed a missing feature | Edits are now checked that they applied |
| Game balanced far too easy | Simulation: 99.9% survival | Added hardship and starvation tolls; retuned thresholds |
| Desperate-event variants almost never or almost always fired | Simulation of variant frequency | Retuned thresholds (wagon 35 to 75, health 55 to 40) |
| Lying text sometimes unchanged | Test T14 | Force at least one swap |
| Scene art overflowed its frame | Screenshots | Clip rectangle |


## 6. How AI output was verified

Done by the AI in its own environment, with results shown to me:
1. Compiled everything with warnings on (`-Wall -Wextra`) and fixed warnings.
2. Wrote and ran automated tests (`tests/logic_tests.cpp`) for rules, loaders, text fit, settings, logging and audio.
3. Ran the balance simulation (`tools/balance_sim.cpp`) with four strategies on each difficulty.
4. Ran the game headless and read screenshots back to check layout.

Done by me:
5. Built and ran every version in Visual Studio 2022 and played it through.
6. Checked the art, sound and text in game and asked for changes when something was wrong.
7. Ran two rounds of human playtesting and decided which changes to make from the feedback.

## 7. Repository evidence

| Commit | Message |
|---|---|
| `587643e` | Add Trail class with defensive events.txt loader |
| `bbf2a29` | Add Day 1 event data (5 parts x 4 choices) |
| `391bbe0` | Add event, result, day summary and ending flow with stat HUD |
| `123164a` | Update Visual Studio project, add CMake support, bundle SDL3 |
| `958ec60` | Update README status for Week 3 |
| `1b3f873` | Add Days 2-3, sanity-driven narrator, wagon scene art, balance simulator and playtesting docs |
| `8a817cf` | 18 Oct | Add PartyMember and Observer pattern: named family, threshold warnings, desperate event variants |
| `c18e610`, `eb41fe3`, `5442df8`, `03cfc81`, `bf5bb5d`, `3038a1c` | 19-22 Oct | My own edits to Button, Conditions, Effects and Event (see AI Development Log for what I changed by hand). `3038a1c` is the build used for playtest round 1. |
| `b4a77f3` | 25 Oct | Make the lying narrator noticeable: trust meter, narrator-shift screen, end-of-day truth check. **Response to round 1** (players did not notice the narrator lying). This is the build used for round 2. |
| `a8f589a` | 28 Oct | Round 2 feedback: add Easy/Normal/Hard difficulty, per-event scenes, per-choice reaction animations, remembered discoveries and pixel-art rendering. **Response to round 2** (difficulty rated 1, no reason to replay). |
| `8fe6f85` | 28 Oct | Update balance-report.md (results for all three difficulties) |


## 8. Ownership: what I can explain

1. Why does Party notify observers instead of calling the watchers directly?
Party keeps a list of anything that implements the PartyObserver interface, and after every change it compares the stats now with a snapshot from before. If something changed, it tells every observer in the list. Party never mentions ThresholdWatcher or MemberWatcher by name, so it works the same whoever is listening. Game registers the watchers in its constructor, and adding a third listener (say, an achievements tracker) would need no change to Party. That’s the Observer pattern: the subject broadcasts, and the listeners decide what to do.

2. How does the State pattern decide how trustworthy the game is, and why can’t it change a choice’s effects?
NarrativeState is a base class with three versions: Stable, Unreliable and Breakdown. Game holds one through a base-class pointer and swaps it when sanity crosses a threshold (60 and 30). Each version implements filterText and shownValue differently: Stable returns the truth, Unreliable swaps some words and nudges HUD numbers, and Breakdown hides numbers and adds asides. These functions only change what’s displayed. The effects are applied by Party using the real numbers, and the “what happened” line is added after the filter, so it’s always true. That separation was a design decision, so the game never cheats.

3. What does the loader do with a malformed event, and why?
Trail::loadFromFile reads the file line by line. If a choice has no label or no result, it records a warning with the file and line number and skips the whole event. Long labels, unknown conditions or members, and duplicate slots are also reported, with unusable entries skipped. The game never crashes on bad data and always says what was wrong. It also copes with a missing or empty file, with odd characters (BOM, Windows line endings, curly quotes), and with out-of-range numbers by clamping them. The tests in T18a-c check all of this.

4. How does a desperate variant replace a normal event?
Each event can have a requires = LowFood line (or LowWagon, LowHealth), which is stored as its condition. ThresholdWatcher keeps a set of which conditions are currently active. When Game asks the Trail for the event in a slot, getEvent looks at everything stored there. If a variant’s condition is active, that variant wins. Otherwise it returns the normal event. So the variant only appears while the party is actually in trouble, and the normal event comes back once things recover.

5. How is health calculated, and what does frailty do?
Each of the four members has their own health out of 100. The party’s Health bar is the average, with the dead counting as 0, so losing someone drags the bar down. Frailty is a percentage that scales damage, never healing: Ma 100%, Pa 90%, Junior 120%, Granny 150%. Damage is multiplied by frailty and rounded up, so the same hardship hurts Granny more than Pa. A member below 40 health is Ill, and at 0 they’re Dead, and the dead take no further changes.

6. What happens if irrKlang or a sound file is missing?
The game keeps running. Without irrKlang compiled in, AudioManager is a silent stand-in whose functions do nothing, and the title screen says sound is unavailable. If irrKlang is built in but there’s no sound device or a file is missing, init records that audio isn’t available and every play call returns harmlessly. Test T22 checks both cases.

## 9. Declaration

Signed Generative AI Declaration form: https://liveplymouthac-my.sharepoint.com/:w:/r/personal/alexander_tovar_students_plymouth_ac_uk/Documents/Generative%20AI%20Tool%20declaration%20template%20-%20All%20-%20v1.0.docx?d=wd999a88252224706b1061d8d156a7792&csf=1&web=1&e=hTR9pb
