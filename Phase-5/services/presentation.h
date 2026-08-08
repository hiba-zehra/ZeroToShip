// services/presentation.h
//
// Presentation Layer -- Phase 4.
//
// Everything up to this point (models, auth, database, gatekeeper,
// book_core) is pure logic: it never prints a single character to the
// screen. This module is the opposite -- it owns *how the terminal looks*
// and knows nothing about where the data actually came from. That's on
// purpose: Phase 4 builds this layer against mock, hardcoded arrays so the
// visuals can be nailed down first; a later phase swaps the mock arrays
// for the real vectors coming out of Database::loadStory() /
// Database::loadPitches() without this file changing at all.
//
// Two responsibilities live here:
//
//   1. Story Canvas Interface
//      renderStoryCanvas() clears the screen and streams the accepted
//      paragraphs as one continuous, cohesive block of text -- the way a
//      reader experiences a finished story, not a list of database rows
//      with ids and authors stapled to them.
//
//   2. Pitch Registry Workspace
//      renderPitchRegistry() prints every candidate pitch as a numbered
//      row inside a uniform ASCII grid, built entirely out of '+' (corners
//      / joins) and '-' (horizontal rules), so an editor can scan
//      competing pitches for a slot at a glance.

#ifndef STORYFORGE_SERVICES_PRESENTATION_H
#define STORYFORGE_SERVICES_PRESENTATION_H

#include <string>
#include <vector>
#include "../models/Paragraph.h"
#include "../models/Pitch.h"

namespace Presentation {

    // Clears the terminal screen using the appropriate platform command
    // (cls on Windows, clear everywhere else). Every render* function below
    // calls this first so each screen is drawn on a blank terminal instead
    // of stacking underneath whatever was printed before it.
    void clearScreen();

    // --- 1. Story Canvas Interface ---------------------------------------

    // Clears the screen, prints a banner, then streams `story` as a single
    // cohesive block of text: paragraphs are joined with a space (not
    // printed as separate numbered/attributed entries) so the result reads
    // like prose. Assumes `story` is already in chronological order (see
    // services/book_core.h's getChronologicalOrder()) -- this function
    // does not re-sort it.
    //
    //   wordDelayMs - milliseconds to pause between each streamed word,
    //                 producing a typewriter-style reveal. Pass 0 for an
    //                 instant, non-animated print (useful for automated
    //                 tests / non-interactive terminals).
    void renderStoryCanvas(const std::vector<Paragraph>& story,
                            const std::string& title = "STORY CANVAS",
                            unsigned int wordDelayMs = 15);

    // --- 2. Pitch Registry Workspace --------------------------------------

    // Renders every pitch in `pitches` as a numbered row (1-based, in
    // display order) inside a bordered ASCII grid. Columns: #, Author,
    // Target slot, Status, and Pitch Text (truncated with "..." if it
    // doesn't fit the column width). Every row is framed top and bottom
    // with a '+'/'-' rule so the whole registry reads as one uniform grid.
    void renderPitchRegistry(const std::vector<Pitch>& pitches,
                              const std::string& title = "PITCH REGISTRY WORKSPACE");

}

#endif // STORYFORGE_SERVICES_PRESENTATION_H
