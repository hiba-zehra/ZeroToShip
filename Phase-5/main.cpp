// main.cpp
//
// Phase 5 — Final System Integration.
//
// This is the piece that was missing: an operational main loop that wires
// the Phase 4 presentation layer directly to the Phase 1-3 engine and
// data-persistence layer. Nothing in here is mock data -- every call goes
// through Database:: (disk), BookCore:: (engine), Gatekeeper:: (access
// control), and Auth:: (session), exactly the modules phase4_demo.cpp was
// deliberately kept isolated from.
//
// Menu:
//   1. Read Story              -> Database::loadStory + BookCore sort
//                                  + Presentation::renderStoryCanvas
//   2. Submit a Pitch          -> Database::appendPitch (real write)
//   3. Review Pitches [Editor] -> Gatekeeper::canModerate gate,
//                                  BookCore::acceptPitchAndPersist
//
// Starter Node Optimization: on boot, if story_db.json is missing/empty,
// we seed a default Paragraph 0 so the very first user has a valid,
// non-empty system state to read and pitch against.

#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "models/Paragraph.h"
#include "models/Pitch.h"
#include "services/auth.h"
#include "services/book_core.h"
#include "services/database.h"
#include "services/gatekeeper.h"
#include "services/presentation.h"

namespace {

const std::string STORY_FILE = "story_db.json";
const std::string PITCH_FILE = "pitches_db.json";

// Reads one full line of input, re-prompting on empty input if `required`.
std::string promptLine(const std::string& label, bool required = true) {
    std::string line;
    while (true) {
        std::cout << label;
        std::getline(std::cin, line);
        if (!required || !line.empty()) return line;
        std::cout << "  (this can't be empty -- try again)\n";
    }
}

// Reads one integer, re-prompting on bad input.
int promptInt(const std::string& label) {
    while (true) {
        std::cout << label;
        std::string line;
        std::getline(std::cin, line);
        try {
            size_t consumed = 0;
            int value = std::stoi(line, &consumed);
            if (consumed == line.size()) return value;
        } catch (...) {
            // fall through to the retry message below
        }
        std::cout << "  (please enter a whole number)\n";
    }
}

void pause() {
    std::cout << "\n(press Enter to continue)";
    std::string discard;
    std::getline(std::cin, discard);
}

// Starter Node Optimization: guarantees the story never boots empty.
// If story_db.json is missing or contains zero paragraphs, we synthesize
// a default opening paragraph and persist it immediately, so the first
// user to run "Read Story" or submit a pitch has real state to work from.
void ensureStorySeeded() {
    std::vector<Paragraph> story = Database::loadStory(STORY_FILE);
    if (!story.empty()) return;

    Paragraph seed(
        /*id=*/1,
        "The glowing terminal screen blinked in the empty computer lab, "
        "waiting for someone -- anyone -- to write the first line.",
        /*author=*/"system",
        /*order_num=*/1);

    if (Database::appendParagraph(seed, STORY_FILE)) {
        std::cout << "(no existing story found -- seeded a starting paragraph)\n";
    } else {
        std::cerr << "(warning: could not seed a starting paragraph -- check "
                     "write permissions in this folder)\n";
    }
}

// Figures out what order_num the *next* pitch should target: one past
// whatever the story's current highest order_num is. Reuses BookCore's
// own definition of "active" so this never drifts out of sync with the
// engine's accept-pitch pipeline.
int nextOpenSlot() {
    std::vector<Paragraph> story = Database::loadStory(STORY_FILE);
    return BookCore::getActiveOrderNum(story) + 1;
}

// The next id to hand a freshly created Pitch. Flat-file storage has no
// auto-increment, so we derive it the same way BookCore expects callers
// to derive nextParagraphId: one past the current max.
int nextPitchId(const std::vector<Pitch>& pitches) {
    int maxId = 0;
    for (const auto& p : pitches) maxId = std::max(maxId, p.id);
    return maxId + 1;
}

// The next id to hand a freshly accepted Paragraph.
int nextParagraphIdFor(const std::vector<Paragraph>& story) {
    int maxId = 0;
    for (const auto& p : story) maxId = std::max(maxId, p.id);
    return maxId + 1;
}

// --- Menu option 1: Read Story -------------------------------------------
void handleReadStory() {
    std::vector<Paragraph> story = Database::loadStory(STORY_FILE);
    std::vector<Paragraph> ordered = BookCore::getChronologicalOrder(story);
    Presentation::renderStoryCanvas(ordered, "STORY CANVAS -- StoryForge");
    pause();
}

// --- Menu option 2: Submit a Pitch ---------------------------------------
void handleSubmitPitch() {
    if (!Auth::isLoggedIn()) {
        std::cout << "\nYou need to be logged in to submit a pitch.\n";
        std::string username = promptLine("Enter a username to log in as: ");
        if (!Auth::login(username)) {
            std::cout << "Login failed -- try again from the main menu.\n";
            pause();
            return;
        }
    }

    int slot = nextOpenSlot();
    std::cout << "\nYour pitch will compete for slot #" << slot
              << " (the next open position in the story).\n";
    std::string text = promptLine("Write your pitch text: ");

    std::vector<Pitch> pitches = Database::loadPitches(PITCH_FILE);
    Pitch pitch(nextPitchId(pitches), slot, text, Auth::getActiveUser());

    if (Database::appendPitch(pitch, PITCH_FILE)) {
        std::cout << "\nPitch submitted for slot #" << slot << ". "
                  << "The current editor can review it from option 3.\n";
    } else {
        std::cerr << "\nSomething went wrong saving your pitch -- it was NOT recorded.\n";
    }
    pause();
}

// --- Menu option 3: Review Pitches [Editor Only] -------------------------
void handleReviewPitches() {
    std::vector<Paragraph> story = Database::loadStory(STORY_FILE);

    if (!Auth::isLoggedIn()) {
        std::cout << "\nYou need to be logged in to review pitches.\n";
        std::string username = promptLine("Enter a username to log in as: ");
        if (!Auth::login(username)) {
            std::cout << "Login failed -- try again from the main menu.\n";
            pause();
            return;
        }
    }

    if (!Gatekeeper::canModerate(story)) {
        std::cout << "\nOnly the author of the most recently accepted "
                     "paragraph can review pitches right now.\n";
        if (!story.empty()) {
            Paragraph latest = BookCore::getChronologicalOrder(story).back();
            std::cout << "Current editor: " << latest.author << "\n";
        }
        pause();
        return;
    }

    std::vector<Pitch> pitches = Database::loadPitches(PITCH_FILE);
    int slot = nextOpenSlot();

    // Only show pitches actually competing for the next open slot --
    // matches what BookCore::acceptPitch will actually validate against.
    std::vector<Pitch> candidates;
    for (const auto& p : pitches) {
        if (p.status == "Pending" && p.target_order_num == slot) {
            candidates.push_back(p);
        }
    }

    Presentation::renderPitchRegistry(
        candidates, "PITCH REGISTRY -- Candidates for Slot #" + std::to_string(slot));

    if (candidates.empty()) {
        std::cout << "\nNothing to review yet for this slot.\n";
        pause();
        return;
    }

    std::cout << "\nEnter the id of the pitch to accept (or 0 to cancel): ";
    int chosenId = promptInt("");
    if (chosenId == 0) {
        pause();
        return;
    }

    int nextId = nextParagraphIdFor(story);
    BookCore::AcceptResult result = BookCore::acceptPitchAndPersist(
        chosenId, pitches, story, nextId, STORY_FILE, PITCH_FILE);

    if (result.success) {
        std::cout << "\nAccepted! New paragraph #" << result.newParagraph.order_num
                  << " added by " << result.newParagraph.author << ".\n";
        if (!result.rejectedPitchIds.empty()) {
            std::cout << result.rejectedPitchIds.size()
                      << " competing pitch(es) for that slot were rejected.\n";
        }
    } else {
        std::cout << "\nCouldn't accept that pitch: " << result.message << "\n";
    }
    pause();
}

void printMenu() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  StoryForge -- Main Menu";
    if (Auth::isLoggedIn()) {
        std::cout << "  (logged in as " << Auth::getActiveUser() << ")";
    }
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  1. Read Story\n";
    std::cout << "  2. Submit a Pitch\n";
    std::cout << "  3. Review Pitches [Editor Only]\n";
    std::cout << "  4. Log out\n";
    std::cout << "  0. Exit\n";
    std::cout << std::string(60, '-') << "\n";
}

} // namespace

int main() {
    ensureStorySeeded();

    bool running = true;
    while (running) {
        printMenu();
        int choice = promptInt("Choose an option: ");

        switch (choice) {
            case 1: handleReadStory(); break;
            case 2: handleSubmitPitch(); break;
            case 3: handleReviewPitches(); break;
            case 4:
                Auth::logout();
                std::cout << "\nLogged out.\n";
                break;
            case 0:
                running = false;
                std::cout << "\nGoodbye -- thanks for writing with StoryForge!\n";
                break;
            default:
                std::cout << "\nNot a valid option -- pick a number from the menu.\n";
        }
    }

    return 0;
}
