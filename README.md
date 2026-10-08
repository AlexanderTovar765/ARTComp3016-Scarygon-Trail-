# The Scarygon Trail

A comedic survival-horror resource-management game inspired by *The Oregon Trail*, built in C++ with SDL3 for COMP3016 (Immersive Game Technologies) Coursework 1.

Every stop on the trail presents a short narrative event, delivered in the same flat, matter-of-fact tone whether it's a broken wheel or something considerably stranger. Manage food, health, sanity and wagon condition, and live with the consequences of your choices.

## Status
Week 2: project skeleton. SDL3 window opens with the split-panel layout (top 60% for 2D visuals, bottom 40% for narrative text and choices). Panels are placeholder colours for now.

## Documentation
- [Week 2 proposal](docs/proposal.pdf)
- [AI Development Log](docs/ai-dev-log.md)

## Build
Requires SDL3.

**Visual Studio 2022:** add the SDL3 include and library paths to the project properties, then build and run.

**g++ (Linux/macOS):**
```
g++ src/main.cpp src/Game.cpp -o ScarygonTrail -lSDL3
```

## Controls
- `Esc`: quit

## Project structure
```
src/      C++ source
assets/   sprites, fonts
data/     event and trail files loaded at runtime
docs/     proposal and AI Development Log
```
