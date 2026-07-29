# Phase 3 — StoryForge: The Book Core Engine (C++ Edition)

## What is this project

StoryForge is a collaborative story-writing tool that runs in the terminal.
People write a story together, one paragraph at a time. Whoever wrote the
most recently accepted paragraph becomes the "editor" for a bit — they
review pitches from other people and pick their favorite one to continue
the story.

Phases 1 and 2 built the foundation: the `Paragraph`/`Pitch` data models
with JSON serialization, flat-file persistence (`services/database.h`),
a lightweight login session (`services/auth.h`), and the access-control
check for who's allowed to moderate right now (`services/gatekeeper.h`).

Phase 3 is where the story actually **moves forward**. It adds
`services/book_core.h` / `.cpp` — the engine that turns an accepted pitch
into a real paragraph in the chain.

## What's actually in here

### 1. Chronological Sorting Engine

`BookCore::getChronologicalOrder(paragraphs)` returns a sorted copy of the
story in reading order. It uses a lambda comparator on `order_num`:

```cpp
std::sort(paragraphs.begin(), paragraphs.end(),
          [](const Paragraph& a, const Paragraph& b) {
              return a.order_num < b.order_num;
          });
```

This is the direct C++ equivalent of Python's
`sorted(paragraphs, key=lambda x: x.order_num)` — paragraphs get appended
to the story in whatever order they were accepted, but readers need to
see them in `order_num` order, not insertion order.

`BookCore::getActiveOrderNum(story)` is the small helper both this and the
pipeline below rely on: the highest `order_num` currently in the story (or
`0` for a brand-new, empty story).

### 2. State Transition Pipeline — `BookCore::acceptPitch()`

This is the "accept pitch" pipeline described in the brief. When a Pitch is
accepted, three things have to happen **together**:

1. A new `Paragraph` is instantiated at `active_order_num + 1`.
2. The winning pitch's `status` flips from `"Pending"` to `"Accepted"`.
3. Every *other* `"Pending"` pitch competing for that same
   `target_order_num` flips to `"Rejected"` — only one pitch per position
   ever wins.

All three happen atomically: `acceptPitch()` validates everything (pitch
exists, is still `"Pending"`, and its `target_order_num` actually matches
the slot the story is expecting) **before** touching any state. If
validation fails, `pitches` and `story` come back completely untouched.

```cpp
BookCore::AcceptResult result = BookCore::acceptPitch(
    pitchId, pitches, story, nextParagraphId);

if (result.success) {
    // result.newParagraph        -> the paragraph that was just created
    // result.rejectedPitchIds    -> ids of the competitors that lost
} else {
    // result.message explains why (unknown id, already resolved,
    // or a target_order_num mismatch)
}
```

### 3. Persistence wrapper — `BookCore::acceptPitchAndPersist()`

A thin wrapper around `acceptPitch()` that also writes the updated story
and pitch list to disk (`story_db.json` / `pitches_db.json`, reusing
`services/database.h`). If either file fails to save, the in-memory
mutation is rolled back — we never want memory and disk to disagree about
whether a pitch was actually accepted.

This required extending `services/database.h`/`.cpp` (Phase 2 only
persisted `Paragraph`s) with matching `loadPitches()` / `savePitches()` /
`appendPitch()` functions, mirroring the paragraph versions exactly.

### 4. The manual test (`manual_test.cpp`)

Same spirit as Phases 1 & 2 — plain PASS/FAIL output, no framework. It
checks:

- The sorting engine produces correct reading order and doesn't mutate
  its input.
- `acceptPitch()` on a valid pending pitch: correct `order_num`, correct
  text/author, correct id assignment, and that competing pitches for the
  same slot flip to `Rejected` while pitches for *other* slots are left
  alone.
- Three failure paths (unknown pitch id, an already-resolved pitch, a
  `target_order_num` that doesn't match the expected slot) all leave
  `pitches` and `story` completely untouched — proving the pipeline is
  actually atomic, not just "usually fine."
- `acceptPitchAndPersist()` round-trips correctly through
  `story_db.json` / `pitches_db.json`.
- `Gatekeeper::canModerate()` still works correctly against a story built
  up entirely through `BookCore`.

## How to build and run it

```bash
make run
```

If `make` isn't available:

```bash
g++ -std=c++17 -I. -Imodels -Iservices -Iexternal manual_test.cpp \
    models/Paragraph.cpp models/Pitch.cpp \
    services/auth.cpp services/database.cpp services/gatekeeper.cpp \
    services/book_core.cpp -o manual_test
./manual_test
```

You should see `ALL CHECKS PASSED (33/33)` at the bottom.

## A note on the Phase 2 Makefile

While wiring this phase up, I noticed the Phase 2 `Makefile`'s `SRCS` only
listed `manual_test.cpp`, `models/Paragraph.cpp`, and `models/Pitch.cpp` —
it never actually compiled `services/auth.cpp`, `services/database.cpp`,
or `services/gatekeeper.cpp`, even though `manual_test.cpp` calls into all
three. I fixed this here by adding every service's `.cpp` file to `SRCS`
and adding `-I.` / `-Iservices` so `services/database.cpp`'s
`#include "../json.hpp"` resolves correctly.

## What I learned doing this

- How to design a small "atomic" operation in a language without database
  transactions: validate everything up front, mutate only after every
  check passes, and roll back manually if a later step (disk I/O) fails.
- Why `active_order_num + 1` needs to be computed fresh each time rather
  than tracked as separate mutable state — it's derived data, and deriving
  it avoids ever having it get out of sync with the actual story.
- Extending an existing module (`services/database.h`) to support a
  second model type by mirroring its existing pattern instead of
  reinventing one.
- Lambda comparators for `std::sort` are basically Python's `key=` argument
  in disguise — same idea, different syntax.
