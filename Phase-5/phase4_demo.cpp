// phase4_demo.cpp
//
// Phase 4 manual verification / walkthrough for services/presentation.h.
//
// Per the Phase 4 brief: this drives the presentation layer with mock,
// hardcoded arrays only -- no Database:: or BookCore:: calls -- so the
// Story Canvas and Pitch Registry visuals can be reviewed on their own
// before anything gets wired to live data in a later phase.
//
// Run with `make phase4` (see Makefile). Pass "--fast" as an argv to skip
// the typewriter delay on the Story Canvas (handy for CI / non-interactive
// runs); interactively it defaults to a small per-word delay.

#include <iostream>
#include <string>
#include <vector>

#include "models/Paragraph.h"
#include "models/Pitch.h"
#include "services/presentation.h"

int main(int argc, char* argv[]) {
    bool fast = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--fast") fast = true;
    }
    unsigned int wordDelayMs = fast ? 0 : 15;

    // --- Mock story: hardcoded Paragraph objects, already in chronological
    // order_num order (as if BookCore::getChronologicalOrder() had already
    // run on them). No file I/O, no BookCore, no Database -- just literals.
    std::vector<Paragraph> mockStory = {
        Paragraph(1, "The lighthouse keeper had not seen another soul in three winters.",
                  "hiba", 1),
        Paragraph(2, "So when a rowboat appeared out of the fog, he assumed it was a trick of the light.",
                  "ahmed", 2),
        Paragraph(3, "It was not a trick. The boat was empty, save for a single sealed letter.",
                  "sara", 3)
    };

    Presentation::renderStoryCanvas(mockStory, "STORY CANVAS -- StoryForge Preview", wordDelayMs);
    std::cout << "\n(press Enter to view the Pitch Registry mock)\n";
    std::cin.get();

    // --- Mock pitches: hardcoded candidates competing for the *next* slot
    // (order_num 4), plus one already-resolved pitch to show status
    // rendering. Again, purely literal data -- nothing loaded from disk.
    std::vector<Pitch> mockPitches = {
        Pitch(101, 4, "He broke the wax seal, hands trembling from the cold.", "hiba"),
        Pitch(102, 4, "He tossed the letter into the sea, unopened -- some things are better left unread.", "ahmed"),
        Pitch(103, 4, "A gull landed on the letter before he could reach it.", "sara"),
        Pitch(104, 3, "An earlier pitch for slot 3 that already lost out.", "ahmed", "Rejected")
    };

    Presentation::renderPitchRegistry(mockPitches, "PITCH REGISTRY -- Candidates for Slot #4");

    std::cout << "\nPhase 4 static layout preview complete.\n";
    return 0;
}
