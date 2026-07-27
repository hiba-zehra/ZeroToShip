# Phase 1 — StoryForge: Data Models & Serialization (C++ Edition)

## What is this project

StoryForge is basically a collaborative story-writing tool that runs in the
terminal. The idea is simple: people write a story together, one paragraph
at a time, instead of everyone writing whatever they want in a big messy
tree of branches. Whoever wrote the most recent accepted paragraph becomes
the "editor" for a bit — they get to look at pitches from other people and
pick their favorite one to continue the story.

This phase doesn't touch any of that logic yet, though. Phase 1 is just
about getting the foundation right — the actual data structures the rest
of the app is going to sit on top of. So there's no menu, no loop asking
the user what they want to do, none of that. Just two classes and a script
to prove they work the way they're supposed to.

## What's actually in here

### 1. The `Paragraph` class (`models/Paragraph.h` / `.cpp`)

This is one accepted block of the story. Pretty straightforward:

| Attribute   | Type          | What it's for                              |
|-------------|---------------|---------------------------------------------|
| `id`        | `int`         | A unique number for this paragraph          |
| `text`      | `std::string` | The actual paragraph content                |
| `author`    | `std::string` | Who wrote it                                |
| `order_num` | `int`         | Where it sits in the story                  |

### 2. The `Pitch` class (`models/Pitch.h` / `.cpp`)

This is someone's attempt at writing the next paragraph — it hasn't been
accepted yet, it's just competing for a spot.

| Attribute          | Type          | What it's for                                  |
|--------------------|---------------|--------------------------------------------------|
| `id`               | `int`         | Unique number for the pitch                     |
| `target_order_num` | `int`         | Which position it's trying to fill              |
| `text`             | `std::string` | The proposed text                                |
| `author`           | `std::string` | Who submitted it                                 |
| `status`           | `std::string` | `Pending`, `Accepted`, or `Rejected`             |

One thing I added on purpose: if you try to set `status` to anything other
than those three exact values, the constructor throws an error instead of
just letting it happen. It felt kind of pointless to let something like
`"acepted"` (typo) or `"maybe"` sneak in and mess up the data later, so I'd
rather catch it right away.

### 3. Serialization — `toJson()` / `fromJson()`

Here's the actual problem serialization solves: objects only exist while
the program is running. The second you close it, everything in memory is
gone. So if we ever want the story to still be there next time someone
opens the app, it has to get written to a file somewhere — that's what
`story_db.json` will eventually be.

JSON is just a convenient format for that — readable, simple, and every
language has good support for it.

- `toJson()` takes an object and turns it into JSON (this is basically
  what Python's `to_dict()` does).
- `fromJson()` does the reverse — takes JSON and rebuilds the object.

C++ doesn't have a built-in dictionary type the way Python does, so I used
[nlohmann/json](https://github.com/nlohmann/json) for this — it's pretty
much the standard JSON library people reach for in C++ projects. It acts a
lot like an ordered map and can convert to/from actual JSON text easily.

```
Object in memory  --toJson()-->   JSON object   --.dump()-->   JSON text (for the file)
Object in memory  <--fromJson()-- JSON object   <--.parse()--  JSON text (from the file)
```

### 4. The manual test (`manual_test.cpp`)

Since we haven't gotten into real testing frameworks yet, this is just a
plain program you run and read the output of yourself. For both classes it
checks a few things:

1. The attributes actually got stored the way they were passed in.
2. `toJson()` gives back exactly the structure you'd expect.
3. `fromJson()` rebuilds something equal to the original object.
4. It survives a full round trip — object → JSON → text string → parsed
   back → object again — to mimic what'll actually happen when this gets
   written to and read from `story_db.json` later. This catches bugs a
   simple in-memory comparison wouldn't.
5. For Pitch specifically: giving it a bad status actually throws an error
   like it's supposed to.

## How to build and run it

This project uses the [nlohmann/json](https://github.com/nlohmann/json)
library for the JSON serialization. To keep things simple and avoid needing
any package manager, the library's single header file is already bundled
inside `external/nlohmann/json.hpp` — nothing extra to install.

Just run:
```bash
make run
```

If `make` isn't available on your system, you can compile directly instead:
```bash
g++ -std=c++17 -Imodels -Iexternal manual_test.cpp models/Paragraph.cpp models/Pitch.cpp -o manual_test
./manual_test
```

## Troubleshooting (for anyone running this on a different machine)

**"make: command not found"**
`make` isn't installed on your system, or you're on Windows where it isn't
included by default. Skip `make` entirely and use the direct `g++` command
shown above instead — it does the exact same thing in one step.

**"g++: command not found"**
There's no C++ compiler installed at all. On Ubuntu/Debian:
```bash
sudo apt-get install g++
```
On Windows, install [MinGW-w64](https://www.mingw-w64.org/) or use WSL
(Windows Subsystem for Linux), which gives you a full Linux environment.
On Mac, install Xcode Command Line Tools:
```bash
xcode-select --install
```

**"fatal error: nlohmann/json.hpp: No such file or directory"**
This means the compiler isn't finding the bundled header. Make sure you're
running the command from *inside* the `Phase-1-Simple` folder (not from
somewhere else), and that the `-Iexternal` flag is included in the command,
exactly as shown above.

**On Windows PowerShell, `./manual_test` doesn't work**
Use `.\manual_test.exe` instead (note the backslash and `.exe`).

That compiles everything and runs the test in one go. If it all works,
you'll see `ALL CHECKS PASSED (10/10)` at the bottom — there's a saved copy
of that output in `Output/test_run_output.txt` too.

## A few decisions I made and why

- **Why use nlohmann/json instead of writing my own JSON parser?** Writing
  a JSON parser from scratch would've been a whole separate project on its
  own, and it wasn't really the point of this phase. I wanted to spend the
  time on getting the models and serialization logic right, not
  reinventing something that already has a solid, well-tested library.
- **Why validate `status` right in the constructor?** Because catching a
  bad value the moment it's created is a lot easier than catching it after
  it's already been written to a file somewhere and corrupted the data.
- **Why a manual test instead of an actual testing framework?** Honestly,
  at this stage it felt more useful to actually see and understand what
  serialization does step by step, rather than have it hidden behind
  assertions from a framework I don't fully understand yet.

## What I learned doing this

- What serialization actually is and why it matters — the gap between an
  object sitting in memory and something that can survive being saved to
  disk.
- Some core C++ OOP stuff: constructors, member initializer lists, static
  factory functions (`fromJson`), and operator overloading (`operator==`).
- How to actually use an external C++ library in a project (nlohmann/json,
  installed through `apt`).
- Why it's worth validating data as early as possible instead of dealing
  with bad data later.
- How to structure a small C++ project properly with header/implementation
  files and a Makefile, instead of just dumping everything into one file.