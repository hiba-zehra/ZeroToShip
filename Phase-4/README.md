# Phase 4 — Presentation Layer (StoryForge CLI)

## Overview
Phase 4 builds the terminal presentation layer for StoryForge, entirely
against mock, hardcoded data -- no live database or BookCore wiring yet.
This keeps the visuals decoupled from data-fetching so the look and feel
can be nailed down first, then swapped to live data in a later phase
without changing this layer at all.

## What Was Built

### 1. Story Canvas Interface
`Presentation::renderStoryCanvas()` clears the terminal and streams the
accepted story paragraphs as a single, continuous block of prose (not a
list of database rows with ids/authors attached). Text is word-wrapped at
60 characters and streamed word-by-word with an optional typewriter delay.

### 2. Pitch Registry Workspace
`Presentation::renderPitchRegistry()` renders every candidate pitch as a
numbered row inside a uniform ASCII grid built from `+` (corners/joins)
and `-` (horizontal rules), showing author, target slot, status, and a
truncated preview of the pitch text.

## Files
- `services/presentation.h` -- public interface for the presentation layer
- `services/presentation.cpp` -- implementation (screen clearing, word
  streaming, ASCII grid rendering)
- `phase4_demo.cpp` -- standalone demo using hardcoded mock `Paragraph`/
  `Pitch` arrays to exercise both renderers
- `Makefile` -- added a `phase4` target to build/run the demo

## How to Run
```powershell
g++ -std=c++17 -Wall -Wextra -I. -Imodels -Iservices phase4_demo.cpp models/Paragraph.cpp models/Pitch.cpp services/presentation.cpp -o phase4_demo.exe
.\phase4_demo.exe
```
Pass `--fast` to skip the typewriter delay (useful for quick/automated runs).

## Output
See the `Output/` folder for screenshots of both the Story Canvas and
Pitch Registry rendering with mock data.

## Notes
- Cross-platform: uses `Sleep()` on Windows and `usleep()` elsewhere for
  the streaming delay, avoiding a `std::thread` dependency that isn't
  reliably available on all MinGW builds.
- No secrets, API keys, or credentials are used anywhere in this phase.
