# Week 5 AI log entries (draft: edit before pasting into docs/ai-dev-log.md)

These are drafts of what to record for Week 5. The brief expects you to be able to **explain every piece of AI-generated code**, so before you paste any entry: read the code it names, run it, change something small yourself, and write what you changed in the "My changes" line. Leave the verification fields honest; blank is better than invented.

Declare the exact tool and plan you used (the proposal said ChatGPT; this Week 5 work was done with Claude, so say so and explain the change). Ask the module leader what wording they want for the tool/tier.

---

## Entry W5-1: Observer pattern (thresholds and desperate events)

- **Date / tool:** ____ / Claude
- **Category of use:** Code generation, design discussion
- **Prompt (summary):** Asked for the proposal's Observer pattern: stat thresholds that trigger warnings and unlock "desperate" event variants.
- **What the AI produced:** `PartyObserver.h` (interface), `ThresholdWatcher`, `MemberWatcher`, `Conditions.h`, the `requires =` key in the events loader, five desperate events in `data/events.txt`.
- **How it works (in my own words):** ______________________________  *(Party notifies observers after every change; ThresholdWatcher turns "dropped into danger" into a one-off warning and keeps a set of active conditions; Trail returns the desperate variant when its condition is active.)*
- **Verification:** unit tests T10-T13 and T16; balance simulation shows each variant appears in 10-52% of Random runs.
- **My changes:** ______________________________  *(e.g. tuned a threshold, reworded a warning, wrote a desperate event.)*
- **Deviations from the proposal:** none; this is the proposal's Observer feature.

## Entry W5-2: PartyMember (four named family members)

- **Category of use:** Code generation, event writing
- **What the AI produced:** `PartyMember` (health, frailty, Healthy/Ill/Dead), Party composition, `member =` targeting on choices, seven targeted choices, portraits in `SceneRenderer::drawPortrait`.
- **How it works:** ______________________________
- **Verification:** tests T03-T08; screenshots of healthy/ill/dead portraits; Random players lose 0.19 family members on average.
- **My changes:** ______________________________
- **Deviation:** the proposal's "cursed" status was left out to keep scope to 30% of the module (Healthy, Ill, Dead only).

## Entry W5-3: Title, instructions and game-over summary

- **Category of use:** Code generation, UI text
- **What the AI produced:** Title/Instructions phases in `Game`, `journeySummary()`, the title-screen art.
- **Verification:** scripted headless playthroughs to every ending and "Play again"; screenshots.
- **My changes:** ______________________________

## Entry W5-4: Sound (irrKlang) and generated audio

- **Category of use:** Code generation; **generated assets**
- **What the AI produced:** `AudioManager` (a thin wrapper that is a silent no-op when irrKlang is absent); `tools/make_sounds.py`, a Python script that synthesises all 11 WAV files (ambient loops and effects) from simple waveforms.
- **Declaration:** the sounds are AI-assisted (written as code by Claude, no third-party samples). irrKlang is third-party and was requested in the proposal; its licence allows DLL redistribution for non-commercial use only.
- **Verification:** script checks peak level, loop seams and expected frequencies; test T22/T23; engine tested against real irrKlang with a null device. **I have not listened to them. Do that and note your opinion here:** ______________________________
- **My changes:** ______________________________

## Entry W5-5: Optional anonymous playtest log

- **Category of use:** Code generation
- **What the AI produced:** `Settings` (reads `data/settings.txt`), `PlaytestLog` (writes CSV, flushed per row, no personal data).
- **Verification:** tests T20-T21, including a check that no personal data is written.
- **My changes:** ______________________________
- **Ethics note for the report:** off by default; testers are told before it records; the file name is random.

---

## Honest note on process (worth including in the report)

Part of this work was lost when the AI tool's workspace reset, and was rebuilt from the conversation record; the rebuild was checked by re-running all tests and the balance simulation, which reproduced the same numbers as before the reset. This is a real example of verifying AI output rather than trusting it.
