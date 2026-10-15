# Playtesting

Evidence for the brief's playtesting requirements: who tested, what was tested, feedback, changes made, and what is still wrong. Fill this in as sessions happen and commit it after each one, so the history shows feedback leading to changes. Anonymise testers (Tester A, B, C...).

## Session log

| Tester | Date | Who (e.g. classmate, friend) | Build (commit) | How long | Got to |
|---|---|---|---|---|---|
| A | | | | | |
| B | | | | | |
| C | | | | | |

## Observation notes

Watch them play without helping. Write down where they hesitate, laugh, re-read, or get confused.

| Tester | Moment (Day / part) | What I saw | Quote or reaction |
|---|---|---|---|
| | | | |

## Questionnaire (after playing)

Rate 1 (low) to 5 (high).

| Question | A | B | C |
|---|---|---|---|
| It was fun | | | |
| The jokes landed | | | |
| It was clear what each choice would do | | | |
| It was too easy / too hard (1 = too easy, 5 = too hard) | | | |
| Choices felt like they mattered | | | |
| The numbers and text lying to me was noticeable | | | |
| The numbers and text lying to me was fun (not just annoying) | | | |

Open questions:
1. Favourite event or moment?
2. Anything confusing or unfair?
3. Did you notice when the game started lying? How did it feel?
4. Any bugs or things that looked wrong?

## Issues found and what I did about them

| Issue | Found by | Fix | Commit |
|---|---|---|---|
| | | | |

## Reflection (summarise for the report)

- **Who participated:**
- **What was tested:**
- **Feedback received:**
- **Changes I implemented as a result:**
- **Remaining limitations:**

## Things worth asking testers about (from the design)

- The sanity thresholds are 60 (Unreliable) and 30 (Breakdown), set in `src/NarrativeState.h`. Do testers notice the change, and does it feel fair?
- Balance numbers are in `src/Party.h` (the per-part toll) and `data/events.txt`. See `docs/balance-report.md` for the simulated difficulty, and compare it with how real testers do.
