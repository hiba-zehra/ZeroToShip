# StoryForge — A Collaborative Story Chain (CLI Edition)

**Final Submission — ZeroToShip Summer Activity 2026 (Phase 5: Final System Integration)**

## Overview

StoryForge is a terminal-based collaborative story-writing tool. Users read
the story so far, pitch a line to continue it, and whoever wrote the most
recently accepted paragraph becomes the "editor" — the only person allowed
to review pending pitches for the next slot and accept one to move the
story forward.

This repository covers the full project, Phase 1 through Phase 5:

| Phase | What it added |
|-------|----------------|
| 1 | `Paragraph` / `Pitch` data models with JSON (de)serialization |
| 2 | Flat-file persistence (`services/database.h`) and a lightweight login session (`services/auth.h`) |
| 3 | The engine (`services/book_core.h`): chronological sorting and the atomic "accept a pitch" pipeline |
| 4 | The presentation layer (`services/presentation.h`): Story Canvas and Pitch Registry, built and reviewed against mock/hardcoded data |
| 5 | **This phase** — `main.cpp`, the operational main loop that replaces every mock array with live calls into `Database::`, `BookCore::`, `Gatekeeper::`, and `Auth::` |

Nothing in `main.cpp` is hardcoded or mocked. Every read and write goes
through the same modules that were unit-tested in earlier phases.

## Tech Stack

- **Language:** C++17
- **JSON:** [nlohmann/json](https://github.com/nlohmann/json) (single-header, vendored as `json.hpp`)
- **Persistence:** flat JSON files on disk (`story_db.json`, `pitches_db.json`, `.session.json`) — no external database or network dependency
- **Build:** GNU Make + g++

## Project Structure

```
Phase-4/
├── main.cpp                 # Phase 5: the integrated CLI application
├── phase4_demo.cpp          # Phase 4: presentation layer demo against mock data (kept for reference)
├── manual_test.cpp          # Phase 3: engine unit tests (PASS/FAIL output)
├── json.hpp                 # vendored nlohmann/json single header
├── Makefile
├── models/
│   ├── Paragraph.h / .cpp   # accepted story blocks
│   └── Pitch.h / .cpp       # proposed continuations
└── services/
    ├── auth.h / .cpp        # login/session tracking
    ├── database.h / .cpp    # story_db.json / pitches_db.json read+write
    ├── gatekeeper.h / .cpp  # "who is allowed to moderate right now" check
    ├── book_core.h / .cpp   # sorting + the atomic accept-pitch pipeline
    └── presentation.h / .cpp# terminal rendering (Story Canvas, Pitch Registry)
```

## Install & Run Locally

**Prerequisites:** a C++17-capable compiler (g++ 9+ or clang++ 10+) and `make`.
No other dependencies — `json.hpp` is already vendored in the repo.

```bash
# From the Phase-4/ directory:
make app
```

This compiles `main.cpp` together with every model and service, produces
the `storyforge` binary, and runs it immediately. To rebuild without
re-running:

```bash
g++ -std=c++17 -Wall -Wextra -I. -Imodels -Iservices -Iexternal \
    main.cpp models/Paragraph.cpp models/Pitch.cpp \
    services/auth.cpp services/database.cpp services/gatekeeper.cpp \
    services/book_core.cpp services/presentation.cpp \
    -o storyforge
./storyforge
```

On first run, if `story_db.json` doesn't exist yet (or is empty), the app
automatically seeds a default opening paragraph so there's always a valid
starting state — no manual setup required.

## Testing the Full Feature Set

Run `./storyforge` and walk through the menu:

1. **Read Story** — renders the current chain as one continuous, typewriter-style
   block of prose (Story Canvas). On a brand-new run this shows the
   auto-seeded opening paragraph.
2. **Submit a Pitch** — if you're not logged in yet, you'll be prompted for
   a username first. Your pitch is saved to `pitches_db.json`, targeting
   whatever the next open slot in the story is.
3. **Review Pitches [Editor Only]** — only works if the currently logged-in
   user is the author of the most recently accepted paragraph (check with
   option 1, or the app will tell you who the current editor is if you're
   blocked). Lists every pending pitch for the next slot in a bordered grid
   and lets you accept one by id — the winner becomes a new paragraph, and
   every other pending pitch for that same slot flips to `Rejected`.
4. **Log out** — clears the active session.
5. **Exit** — quits the app.

To verify persistence: submit a pitch, exit the app, reopen it, and confirm
the pitch is still there under option 3 — nothing lives only in memory.

To verify the engine's unit tests still pass independently of the CLI:

```bash
make run
```

should print `ALL CHECKS PASSED (33/33)`.

## Reflection

- **Biggest challenge:** keeping `main.cpp` a thin controller — every rule
  (who can moderate, how ids are assigned, what counts as a valid
  transition) already lived in `Gatekeeper::` / `BookCore::` / `Database::`
  from earlier phases, so the job here was wiring and menu flow, not new
  business logic.
- **How modular development helped:** because `services/presentation.h`
  was built and tested against mock data in Phase 4, hooking it up to real
  data in Phase 5 required zero changes to that file — only `main.cpp`
  needed to exist.
- **Future scope:** multi-story support (more than one `story_db.json`),
  concurrent-editor conflict handling, and a richer terminal UI via
  `ncurses`/`curses` for live refresh instead of static screen redraws.
