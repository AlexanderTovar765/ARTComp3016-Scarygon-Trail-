# Test cases

Automated logic tests live in `tests/logic_tests.cpp` (no framework, plain C++; each test group is printed with its checks). They cover the rules and the defensive file loading, not the graphics. Run them from the repository root, because they read `data/` and `assets/audio/`:

```
cmake -S . -B build -DSDL3_DIR=<path to SDL3 cmake config>   # Linux/macOS; on Windows open the folder in Visual Studio
cmake --build build --target logic_tests
ctest --test-dir build --output-on-failure
```

The executable exits with 0 only when every check passes.

**Latest run:** 22,238 checks across 33 test groups, which means that there were no failures with either the silent audio stub or the real irrKlang engine (the sound device). Two failures noticed during these checks have been beneficial: the lying-text check showed that state "lying" can sometimes do absolutely nothing (while nowadays it can change at least one word), and the only mistake regarding hardship has been in calculations (members lose an average of 4 points of health, not 3).

| ID | What is checked | Type | Result |
|---|---|---|---|
| T01 | Effects::describe | Automated | Pass |
| T02 | parsePart / parseCondition accept sensible spellings and reject nonsense | Automated | Pass |
| T03 | PartyMember: frailty, healing cap, permanent death, status thresholds | Automated | Pass |
| T04 | Party starts full; Health is the average of the members (dead count as 0) | Automated | Pass |
| T05 | advanceTime: the trail's toll, and fewer mouths eat less | Automated | Pass |
| T06 | Starvation: at 0 food every living member takes damage each part | Automated | Pass |
| T07 | Targeted effects hit one member; untargeted hit everyone alive; unknown name is ignored | Automated | Pass |
| T08 | Grief: each death costs the survivors sanity | Automated | Pass |
| T09 | Game-over conditions and cause of death | Automated | Pass |
| T10 | ThresholdWatcher: flags track low stats; warnings only on a drop INTO danger | Automated | Pass |
| T11 | MemberWatcher: illness, recovery and death messages | Automated | Pass |
| T12 | Several observers, removal, and copies do not inherit observers | Automated | Pass |
| T13 | Party::reset notifies observers (so flags recover) | Automated | Pass |
| T14 | NarrativeState: thresholds, determinism, honest/lying behaviour | Automated | Pass |
| T15 | The shipped data/events.txt loads cleanly with every slot filled | Automated | Pass |
| T16 | Desperate variants replace the normal event only while their condition is active | Automated | Pass |
| T17 | Text fits its panel: honest, and in the worst lying state; labels fit their buttons | Automated | Pass |
| T18a | Missing, empty and wrong files never crash and always say why | Automated | Pass |
| T18b | A malformed event is skipped with a warning; good events survive | Automated | Pass |
| T18c | Duplicate detection, variants, and a slot with only variants | Automated | Pass |
| T19 | Position tracking: 15 parts, then the next day does not exist | Automated | Pass |
| T20 | Settings: defaults, on/off spellings, bad lines reported, missing file fine | Automated | Pass |
| T21 | PlaytestLog: anonymous CSV, quoting, closes cleanly, fails safely | Automated | Pass |
| T22 | AudioManager is harmless without init, with missing files, and (if built in) with irrKlang | Automated | Pass |
| T23 | Every sound file the game asks for exists | Automated | Pass |
| T24 | LieTracker finds the first swapped word or added aside between true and shown text | Automated | Pass |
| T25 | LieTracker daily and journey counters, report wording and reset | Automated | Pass |
| T26 | LieTracker agrees with the real narrator on every event and result text; Stable never lies; bar lies are counted once per screen | Automated | Pass |
| T27 | Difficulty: Easy is gentler than Normal, Normal gentler than Hard; names, parsing, cycling | Automated | Pass |
| T28 | Party::adjusted scales harm and help, never to zero, never changing who is hit | Automated | Pass |
| T29 | Animation names: every scene and action round-trips, parsing forgives case and dashes | Automated | Pass |
| T30 | The shipped events all have an animation, and every animation is used somewhere | Automated | Pass |
| T31 | A bad scene or action name only warns: the event and its choices still load | Automated | Pass |
| T32 | Progress: endings are classified, discoveries count once, and everything round-trips through the file | Automated | Pass |
| T33 | Progress: a missing, damaged or unwritable file never breaks the game | Automated | Pass |

## Manual and visual checks

| Check | How | Result |
|---|---|---|
| Title, instructions, event, result and ending screens lay out correctly | Headless run with `SDL_VIDEODRIVER=dummy`, screenshots read back with `SDL_RenderReadPixels` | Pass |
| Full playthrough from title to ending and "Play again" | Scripted button presses (always choice 1, 2, 3, 4) | Pass, all four reach an ending and restart cleanly |
| Longest text fits the bottom panel in the worst lying state | T17 plus screenshots | Pass |
| Difficulty button on the title and ending screens cycles Easy / Normal / Hard and the HUD tag follows | Scripted headless playthrough on each difficulty, screenshots read back | Pass, all three reach the ending |
| Every event scene and every choice reaction animation draws and stays inside the picture | Gallery of all 15 scenes and 20 actions rendered to images and checked by eye | Pass |
| Pixel-art rendering keeps thin shapes (hills, people, wheels) and text readable | Screenshots of scenes and reactions after the change | Pass (first version lost thin shapes; fixed) |
| Remembered progress survives a restart, and a missing or damaged `save/progress.txt` is harmless | T32, T33 and a headless run with the file deleted | Pass |
| Playtest log writes one anonymous CSV per session | Ran the playthrough with `playtest_log = on` | Pass |
| Sounds are pleasant and not too loud | Test with users during playtesting | Pass |
| Release build runs on an SMB102 lab machine | Go to Smeaton, log on and run | Pass |

## Balance simulation

`tools/balance_sim.cpp` plays 20,000 games per strategy on each difficulty (Easy, Normal, Hard); results are in `balance-report.md`.

## Known issue

With the real irrKlang engine on the Linux test machine, the test program crashed at exit in roughly 1 run in 10. valgrind shows a use-after-free inside irrKlang's own thread during `engine->drop()`, not in the game's code, so it is recorded here and not "fixed". It does not affect the game on Windows.
