# Playtesting

Evidence for the brief's playtesting requirements: who tested, what was tested, the feedback received, the changes made and what is still wrong. Testers are anonymised (Tester A, B, C...). Each round is committed after the session, so the history shows feedback leading to changes.

## Plan

| Round | Who | Build | Aim | Dates |
|---|---|---|---|---|
| 1 | 2 testers (classmates, friends,) | PLACEHOLDER: commit hash | Find confusing text, unfair moments and bugs in the current game | Oct 23-24 |
| 2 | 3 testers, including at least one from round 1 | PLACEHOLDER: commit hash | Check the round 1 fixes worked and nothing new broke | Oct 30- Nov 2 |

How a session runs: about 15-20 minutes. The tester plays without help while I fill in the observation sheet, then they fill in the questionnaire alone, then I ask what the best moment was and what they would change first. The forms are in `docs/playtest/`.

## Automated and developer testing (not human playtesting)

These are real results, but they test the rules and the code, not how people react to the game.

| What | How | Result |
|---|---|---|
| Difficulty and balance | `tools/balance_sim.cpp` plays 20,000 games per strategy (Random, Greedy, Cautious, Reckless) | Random play survives 67.1% of runs and loses 0.19 family members on average. Greedy and Cautious survive 100%. Reckless survives 0%. See [balance-report.md](balance-report.md) |
| Rules and loaders | `tests/logic_tests.cpp`, 25 test groups | 14,202 checks, all passing. See [test-cases.md](test-cases.md) |
| Layout and flow | Scripted headless playthroughs to every ending, with screenshots | Pass |
| My own play testing | PLACEHOLDER: dates, what you found and fixed | PLACEHOLDER |

## Session log

| Tester | Date | Who | Build (commit) | Time played | Got to |
|---|---|---|---|---|---|
| A | PLACEHOLDER | PLACEHOLDER | PLACEHOLDER | PLACEHOLDER | PLACEHOLDER |
| B | | | | | |
| C | | | | | |
| D | | | | | |
| E | | | | | |

## Observation notes

Written while watching, without helping.

| Tester | Moment (Day / part) | What I saw | Quote or reaction | Issue # |
|---|---|---|---|---|
| PLACEHOLDER | | | | |

## Questionnaire results

Rated 1 (strongly disagree) to 5 (strongly agree). 

| Question | A | B | C | D | E |
|---|---|---|---|---|---|
| It was fun | | | | | |
| The jokes made me laugh | | | | | |
| I understood what to do without help | | | | | |
| It was clear what each choice would do | | | | | |
| The text was easy to read | | | | | |
| The pictures and portraits helped | | | | | |
| The sound fitted the game | | | | | |
| I noticed when the game started lying to me | | | | | |
| The lying was funny or interesting | | | | | |
| My choices felt like they mattered | | | | | |
| I would play it again | | | | | |
| Difficulty (too easy / about right / too hard) | | | | | |

Open-answer themes: PLACEHOLDER: the best moments, the most confusing moments and what testers would change first.

## Feedback to change

Each piece of feedback becomes a GitHub Issue and is closed by a commit.

| Feedback (tester) | Issue | Change made | Commit |
|---|---|---|---|
| PLACEHOLDER | | | |

## Playtesting reflection

- **Who participated:** PLACEHOLDER: number of testers, who they were (anonymised) and when.
- **What was tested:** PLACEHOLDER
- **Feedback received:** PLACEHOLDER
- **Changes implemented:** PLACEHOLDER
- **Remaining limitations:** PLACEHOLDER

## Optional anonymous playtest log

The game can write a small CSV of the choices a playtester made. It is off unless enabled: set `playtest_log = on` in `data/settings.txt` (or press `L` on the title screen). Each session then writes `playtest_logs/session_<random id>.csv`. Tell testers it is on before they play.

- It records: seconds since start, which event and variant, which choice number and label, the four stats after the choice, the narrator state, and START / END / QUIT rows with the result.
- It does not record: names, accounts, device details or typed text. The file name is a random id, and the only date is the UTC day in the header.
- Use it to compare what testers did with what they said in the questionnaire, for example which desperate events they met and how often each ending was reached. Copy interesting rows into the observation notes, and delete the file if the tester asks.
