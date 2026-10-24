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
| My own play testing | 19/10/26-Error with buttons | debugged and now work properly |

## Session log

| Tester | Date | Who | Build (commit) | Time played | Got to |
|---|---|---|---|---|---|
| A | 23/10/26 | User 1 (friend) | PLACEHOLDER | 3min 44sec | Day 3 End |
| B | 23/10/26 | User 2 (friend) | | 2min 15sec | Day 3 End |
| C | | | | | |
| D | | | | | |
| E | | | | | |

## Observation notes

Written while watching.

| Tester | Moment (Day / part) | What I saw | Quote or reaction | 
|---|---|---|---|---|
| Myself | Day 2 Part 1 | User 1 noticed that the choice they made at the same time the previous day lead to the choice theyre at now. | "Oh thats cause i talked to the cows instead of feeding them. |  

## Questionnaire results

Rated 1 (strongly disagree) to 5 (strongly agree). 

| Question | A | B | C | D | E |
|---|---|---|---|---|---|
| It was fun | 4 | 3 | | | |
| The jokes made me laugh | 3 | 3 | | | |
| I understood what to do without help | 5 | 5 | | | |
| It was clear what each choice would do | 4 | 4 | | | |
| The text was easy to read | 5 | 5 | | | |
| The pictures and portraits helped | 4 | 3 | | | |
| The sound fitted the game | 4 | 4 | | | |
| I noticed when the game started lying to me | 2 | 2 | | | |
| The lying was funny or interesting | 3 | 3 | | | |
| My choices felt like they mattered | 4 | 3 | | | |
| I would play it again | 4 | 3 | | | |
| Difficulty (too easy 1-2 / about right 3 / too hard 4-5) | 1 | 1 | | | |


## Feedback to change

| Feedback (tester) | Issue | Change made | Commit |
|---|---|---|---|
| First round of users overall enjoyed the game and were able to finish it. | The game lying to the players was not picked up on | Make the lying text more apparant either with sarcastic phrases or something more blatant | Committed October 24th |

## Playtesting reflection

- **Who participated:** 5 Playtesters over 2 weeks, 5 friends and 1 from class.
- **What was tested:** One full playthrough of the game at their own pace, no help
- **Feedback received:** All my testers liked the game and the quirky style, with biggest complaints coming from lying text not being more exemplified and PLACEHOLDER.
- **Changes implemented:** The lying text was PLACEHOLDER.
- **Remaining limitations:** PLACEHOLDER

