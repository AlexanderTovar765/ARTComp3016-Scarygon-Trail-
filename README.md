# The Scarygon Trail

A comedic survival-horror resource-management game inspired by *The Oregon Trail*, built in C++ with SDL3 for COMP3016 (Immersive Game Technologies) Coursework 1.

Every stop on the trail presents a short narrative event, delivered in the same flat, matter-of-fact tone whether it's a broken wheel or something considerably stranger. Manage food, health, sanity and wagon condition, and live with the consequences of your choices.

## Status
Week 2: SDL3 window with the split-panel layout (top 60% for 2D visuals, bottom 40% for narrative text and choices). The bottom panel shows word-wrapped narrative text and numbered choice buttons (click or press 1-4) using placeholder events. The top panel is still a placeholder colour.

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
data/     event and trail files loaded at runtime
docs/     proposal and AI Development Log
```
