# Balance report

Source: `data/events.txt` (20 events, 3 days, including desperate variants). 20000 simulated runs per strategy and difficulty, fixed random seed.

Health, Sanity or Wagon at 0 ends a run. The difficulties differ only in these rules (see `src/Difficulty.cpp`):

| Difficulty | Start food | Food eaten per part | Sanity lost per part | Wagon lost per part | Hardship (health) per part | Starvation damage | Harm from choices | Help from choices |
|---|---|---|---|---|---|---|---|---|
| Easy | 70 | 3 | 3 | 2 | 3 | 10 | 100% | 100% |
| Normal | 60 | 3 | 4 | 2 | 3 | 10 | 115% | 100% |
| Hard | 50 | 4 | 5 | 2 | 3 | 11 | 125% | 100% |

# Easy

## Survival by strategy

| Strategy | Survives all days | Starved at some point | Reached Unreliable | Reached Breakdown | Saw a desperate event | Avg family lost |
|---|---|---|---|---|---|---|
| Random (picks any choice) | 88.9% | 5.5% | 99.0% | 63.8% | 72.3% | 0.16 |
| Greedy (best total effect) | 100.0% | 0.0% | 100.0% | 0.0% | 100.0% | 0.00 |
| Cautious (protects the lowest stat) | 100.0% | 0.0% | 100.0% | 0.0% | 100.0% | 0.00 |
| Reckless (worst total effect) | 0.0% | 0.0% | 100.0% | 100.0% | 100.0% | 0.00 |

### Random (picks any choice)

Time spent in each narrator state: Stable 56.1%, Unreliable 31.2%, Breakdown 12.6%.

Average final stats of survivors: Food 25, Health 36, Sanity 30, Wagon 68.

Desperate variants seen (share of runs):

- Day 2 Mid-Morning (LowWagon): 10.0%
- Day 2 Noon (LowFood): 14.3%
- Day 3 Dawn (LowWagon): 31.3%
- Day 3 Evening (LowHealth): 21.4%
- Day 3 Noon (LowFood): 34.1%

Family members alive at the end: 4 alive 84.6%, 3 alive 15.2%, 2 alive 0.3%, 1 alive 0.0%, 0 alive 0.0%.

Deaths (2228 of 20000 runs):

- 0.0% of all runs: Everyone has died. The oxen continue without you, out of habit.
- 0.0% of all runs: The wagon has ceased to be a wagon and is now a pile of opinions about wagons.
- 11.1% of all runs: Your sanity has left the wagon. It did not say where it was going.
- died on Day 1: 0.0% of all runs
- died on Day 2: 0.0% of all runs
- died on Day 3: 11.1% of all runs

### Cautious (protects the lowest stat)

Time spent in each narrator state: Stable 73.3%, Unreliable 26.7%, Breakdown 0.0%.

Average final stats of survivors: Food 54, Health 60, Sanity 48, Wagon 50.

Desperate variants seen (share of runs):

- Day 3 Dawn (LowWagon): 100.0%

Family members alive at the end: 4 alive 100.0%, 3 alive 0.0%, 2 alive 0.0%, 1 alive 0.0%, 0 alive 0.0%.

Deaths (0 of 20000 runs):

# Normal

## Survival by strategy

| Strategy | Survives all days | Starved at some point | Reached Unreliable | Reached Breakdown | Saw a desperate event | Avg family lost |
|---|---|---|---|---|---|---|
| Random (picks any choice) | 48.3% | 15.6% | 100.0% | 92.7% | 83.3% | 0.30 |
| Greedy (best total effect) | 100.0% | 0.0% | 100.0% | 0.0% | 100.0% | 0.00 |
| Cautious (protects the lowest stat) | 100.0% | 0.0% | 100.0% | 0.0% | 100.0% | 0.00 |
| Reckless (worst total effect) | 0.0% | 0.0% | 100.0% | 100.0% | 100.0% | 1.00 |

### Random (picks any choice)

Time spent in each narrator state: Stable 48.9%, Unreliable 28.3%, Breakdown 22.8%.

Average final stats of survivors: Food 14, Health 30, Sanity 18, Wagon 60.

Desperate variants seen (share of runs):

- Day 2 Mid-Morning (LowWagon): 13.1%
- Day 2 Noon (LowFood): 46.2%
- Day 3 Dawn (LowWagon): 38.3%
- Day 3 Evening (LowHealth): 28.2%
- Day 3 Noon (LowFood): 52.9%

Family members alive at the end: 4 alive 74.5%, 3 alive 22.1%, 2 alive 2.6%, 1 alive 0.6%, 0 alive 0.2%.

Deaths (10334 of 20000 runs):

- 0.2% of all runs: Everyone has died. The oxen continue without you, out of habit.
- 0.1% of all runs: The wagon has ceased to be a wagon and is now a pile of opinions about wagons.
- 51.3% of all runs: Your sanity has left the wagon. It did not say where it was going.
- died on Day 1: 0.0% of all runs
- died on Day 2: 3.6% of all runs
- died on Day 3: 48.1% of all runs

### Cautious (protects the lowest stat)

Time spent in each narrator state: Stable 40.0%, Unreliable 60.0%, Breakdown 0.0%.

Average final stats of survivors: Food 40, Health 30, Sanity 45, Wagon 38.

Desperate variants seen (share of runs):

- Day 3 Dawn (LowWagon): 100.0%

Family members alive at the end: 4 alive 100.0%, 3 alive 0.0%, 2 alive 0.0%, 1 alive 0.0%, 0 alive 0.0%.

Deaths (0 of 20000 runs):

# Hard

## Survival by strategy

| Strategy | Survives all days | Starved at some point | Reached Unreliable | Reached Breakdown | Saw a desperate event | Avg family lost |
|---|---|---|---|---|---|---|
| Random (picks any choice) | 5.8% | 46.5% | 100.0% | 99.8% | 91.3% | 0.61 |
| Greedy (best total effect) | 100.0% | 100.0% | 100.0% | 100.0% | 100.0% | 1.00 |
| Cautious (protects the lowest stat) | 100.0% | 0.0% | 100.0% | 100.0% | 100.0% | 1.00 |
| Reckless (worst total effect) | 0.0% | 0.0% | 100.0% | 100.0% | 100.0% | 0.00 |

### Random (picks any choice)

Time spent in each narrator state: Stable 47.9%, Unreliable 28.1%, Breakdown 24.0%.

Average final stats of survivors: Food 4, Health 18, Sanity 10, Wagon 46.

Desperate variants seen (share of runs):

- Day 2 Mid-Morning (LowWagon): 17.9%
- Day 2 Noon (LowFood): 83.1%
- Day 3 Dawn (LowWagon): 36.1%
- Day 3 Evening (LowHealth): 17.0%
- Day 3 Noon (LowFood): 31.0%

Family members alive at the end: 4 alive 63.5%, 3 alive 21.0%, 2 alive 9.1%, 1 alive 3.9%, 0 alive 2.5%.

Deaths (18839 of 20000 runs):

- 2.5% of all runs: Everyone has died. The oxen continue without you, out of habit.
- 0.2% of all runs: The wagon has ceased to be a wagon and is now a pile of opinions about wagons.
- 91.5% of all runs: Your sanity has left the wagon. It did not say where it was going.
- died on Day 1: 0.0% of all runs
- died on Day 2: 24.6% of all runs
- died on Day 3: 69.6% of all runs

### Cautious (protects the lowest stat)

Time spent in each narrator state: Stable 33.3%, Unreliable 46.7%, Breakdown 20.0%.

Average final stats of survivors: Food 21, Health 15, Sanity 26, Wagon 27.

Desperate variants seen (share of runs):

- Day 3 Dawn (LowWagon): 100.0%
- Day 3 Evening (LowHealth): 100.0%

Family members alive at the end: 4 alive 0.0%, 3 alive 100.0%, 2 alive 0.0%, 1 alive 0.0%, 0 alive 0.0%.

Deaths (0 of 20000 runs):


## Notes and limits

- Strategies are simple rules, not real players, so treat these numbers as a guide to relative difficulty.
- The simulation ignores the narrator's lies (only the display changes, never the effects) and playtesters' reading of the text.
- Generated by `tools/balance_sim.cpp`; rerun it after editing `data/events.txt`.

## What Week 5 changed (and why)

*These figures describe the game as it was in Week 5, before difficulty levels existed (one rule set, now roughly today's Normal minus its extra harm).*

- **Four named family members** (Ma, Pa, Junior, Granny) replace the single shared Health stat. Health on the HUD is now the average of the members, and the dead count as 0. Frailty differs per person (Ma 100%, Pa 90%, Junior 120%, Granny 150% damage taken), so the same hardship hurts the family unevenly.
- **Hardship toll:** every living member loses 3 health per part (scaled by frailty). The first simulations without it let a Random player survive 99.9% of runs, which is too easy for a game about things going wrong.
- **Fewer mouths eat less:** the food toll shrinks as members die, so losing someone has a small upside as well as the sanity cost (8 per death).
- **Observer thresholds** (Food 25, Wagon 75, Health 40) were tuned so that a Random player meets a desperate event in 81% of runs, but each individual variant appears in 10-52% of runs. Wagon 75 and Health 40 came from earlier values (35 and 55) that made two variants almost never or almost always appear.
- **Targeted choices:** seven choices hurt one member only (for example "Challenge the deer to a duel" costs Pa 20 health).

## What changed after playtest round 2 (and why)

- **The problem:** all of round 2's testers rated the difficulty 1 (too easy), including one who did not reach the end. The simulation above agrees with them for Random play: the old single setting let 67% of Random runs finish, and any careful player finished every time.
- **Three difficulties** (`src/Difficulty.*`) that change only numbers: starting food, how fast food, sanity and the wagon wear down, and how hard bad choices hit (Easy 100%, Normal 115%, Hard 125% of the damage written in `events.txt`). The story, the events and the narrator's lying are identical on every level, and the result text shows the adjusted numbers so the display never lies about what a choice did.
- **Tuning targets** (found with a parameter sweep in `tools/balance_sim.cpp`):
  - **Easy:** a Random player usually finishes (88.9%), a careful one always does with the whole family.
  - **Normal:** harder than the old game. Random finishes about half the time (48.3%) and often loses someone (average 0.30 family lost).
  - **Hard:** Random almost never finishes (5.8%), starvation is a real threat (46.5% starve) and even the Cautious strategy finishes with one family member lost. Greedy and Cautious still finish, so it stays possible for a thoughtful player.
- **Reckless** never finishes on any level, as intended.
- **Replay value:** each event and choice now has its own animation, and the game remembers which animations and endings the player has seen. These do not affect balance, so the simulation does not model them.

## How to read this

The target was: a thoughtful player (Greedy/Cautious) should be able to finish, a careless one (Random) should sometimes or usually finish but often scarred, and a deliberately terrible one (Reckless) should never finish. Each difficulty moves the careless player: Random survival is 88.9% on Easy, 48.3% on Normal and 5.8% on Hard, while Cautious and Greedy still finish on all three (on Hard they lose a family member). Real playtesters (see `playtesting.md`) are the check on whether it also *feels* fair: round 1 and 2 testers found the original game too easy, and the difficulty levels are the response, to be re-checked in any later round.
