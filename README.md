# The Scarygon Trail

A comedic survival-horror resource-management game inspired by *The Oregon Trail*, built in C++ with SDL3 for COMP3016 (Immersive Game Technologies) Coursework 1.

Every stop on the trail presents a short narrative event, delivered in the same flat, matter-of-fact tone whether it's a broken wheel or something considerably stranger. Manage food, health, sanity and wagon condition, and live with the consequences of your choices.

## Status
Week 3: one full playable day. Each day has 5 parts (Dawn, Mid-Morning, Noon, Evening, Night) with one event per part and 4 choices per event. Every choice changes the party's Food, Health, Sanity and Wagon, shown as bars in the top panel. Events are loaded from `data/events.txt`. The game ends if Health, Sanity or the Wagon reaches 0 (or at the end of the available days). The top panel's wagon art is still a placeholder.

## Documentation
- [Week 2 proposal](docs/proposal.pdf)
- [AI Development Log](docs/ai-dev-log.md)

## Build
Requires Visual Studio 2022 with the "Desktop development with C++" workload. SDL3 (3.4.18, x64) is bundled in `vendored/SDL3`, so no separate install is needed.

1. Open `ScarygonTrail.sln`.
2. Select `Debug | x64` (or `Release | x64`).
3. Press F5 to build and run. `SDL3.dll` is copied next to the executable automatically; output goes to `build/`.

## Controls
- Mouse click or `1`-`4`: pick a choice
- `Esc`: quit

## Project structure
```
src/      C++ source
vendored/ bundled SDL3 headers and x64 libraries
assets/   sprites, fonts
data/     events.txt, loaded at runtime (format explained at the top of the file)
docs/     proposal and AI Development Log
```
