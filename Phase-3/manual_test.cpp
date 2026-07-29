// manual_test.cpp
//
// Phase 3 manual verification program (same spirit as Phases 1 & 2 --
// no testing framework, just readable PASS/FAIL output). Covers the new
// services/book_core.h engine: chronological sorting + the atomic
// accept-a-pitch pipeline, on top of the auth/database/gatekeeper checks
// carried over from Phase 2.

#include <iostream>
#include <cstdio>

#include "models/Paragraph.h"
#include "models/Pitch.h"
#include "services/auth.h"
#include "services/database.h"
#include "services/gatekeeper.h"
#include "services/book_core.h"

static int g_pass_count = 0;
static int g_fail_count = 0;

bool check(const std::string& label, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << label << "\n";
        ++g_pass_count;
    } else {
        std::cout << "[FAIL] " << label << "\n";
        ++g_fail_count;
    }
    return condition;
}

int main() {
    const std::string DB_FILE = "test_story_db.json";
    const std::string PITCH_FILE = "test_pitches_db.json";
    std::remove(DB_FILE.c_str());
    std::remove(PITCH_FILE.c_str());
    std::remove(".session.json");

    std::cout << std::string(60, '=') << "\n";
    std::cout << "StoryForge Phase 3 - Manual Verification (Book Core)\n";
    std::cout << std::string(60, '=') << "\n";

    std::cout << "\n--- Testing Chronological Sorting Engine ---\n";
    std::vector<Paragraph> shuffled = {
        Paragraph(10, "third paragraph", "hiba", 3),
        Paragraph(11, "first paragraph", "hiba", 1),
        Paragraph(12, "second paragraph", "ahmed", 2)
    };
    auto ordered = BookCore::getChronologicalOrder(shuffled);
    check("sorted story has 3 paragraphs", ordered.size() == 3);
    check("paragraph at index 0 has order_num 1", ordered.size() > 0 && ordered[0].order_num == 1);
    check("paragraph at index 1 has order_num 2", ordered.size() > 1 && ordered[1].order_num == 2);
    check("paragraph at index 2 has order_num 3", ordered.size() > 2 && ordered[2].order_num == 3);
    check("original vector passed in was untouched", shuffled[0].order_num == 3);

    check("getActiveOrderNum() is 0 for an empty story",
          BookCore::getActiveOrderNum({}) == 0);
    check("getActiveOrderNum() returns the highest order_num",
          BookCore::getActiveOrderNum(shuffled) == 3);

    std::cout << "\n--- Testing acceptPitch() (in-memory pipeline) ---\n";
    std::vector<Paragraph> story; // empty story -> next paragraph is order_num 1
    std::vector<Pitch> pitches = {
        Pitch(1, 1, "Once upon a time...", "hiba"),
        Pitch(2, 1, "It was a dark and stormy night...", "ahmed"),
        Pitch(3, 1, "In a galaxy far away...", "sara"),
        Pitch(4, 2, "The next chapter begins...", "ahmed") // targets a slot that doesn't exist yet
    };
    int nextParagraphId = 100;

    auto result = BookCore::acceptPitch(1, pitches, story, nextParagraphId);
    check("acceptPitch() reports success for a valid, pending pitch", result.success);
    check("new paragraph gets order_num 1 (active_order_num 0 + 1)",
          result.newParagraph.order_num == 1);
    check("new paragraph text/author come from the winning pitch",
          result.newParagraph.text == "Once upon a time..." &&
          result.newParagraph.author == "hiba");
    check("new paragraph got the caller-supplied id (100)", result.newParagraph.id == 100);
    check("nextParagraphId was incremented after success", nextParagraphId == 101);
    check("story now has exactly 1 paragraph", story.size() == 1);

    Pitch* p1 = nullptr, *p2 = nullptr, *p3 = nullptr, *p4 = nullptr;
    for (auto& p : pitches) {
        if (p.id == 1) p1 = &p;
        if (p.id == 2) p2 = &p;
        if (p.id == 3) p3 = &p;
        if (p.id == 4) p4 = &p;
    }
    check("winning pitch (id 1) flipped to Accepted", p1 && p1->status == "Accepted");
    check("competing pitch (id 2, same slot) flipped to Rejected", p2 && p2->status == "Rejected");
    check("competing pitch (id 3, same slot) flipped to Rejected", p3 && p3->status == "Rejected");
    check("pitch for a different slot (id 4) left Pending", p4 && p4->status == "Pending");
    check("rejectedPitchIds reports exactly the two competitors",
          result.rejectedPitchIds.size() == 2);

    std::cout << "\n--- Testing acceptPitch() failure cases (atomicity) ---\n";
    auto snapshotStorySize = story.size();
    auto snapshotPitchesCount = pitches.size();

    auto badId = BookCore::acceptPitch(999, pitches, story, nextParagraphId);
    check("acceptPitch() fails for an unknown pitch id", !badId.success);
    check("story untouched after failed call (bad id)", story.size() == snapshotStorySize);

    auto alreadyResolved = BookCore::acceptPitch(2, pitches, story, nextParagraphId);
    check("acceptPitch() fails for a pitch that's already Rejected", !alreadyResolved.success);
    check("story untouched after failed call (already resolved)", story.size() == snapshotStorySize);

    // Pitch 4 targets order_num 2, but the story is empty of anything at
    // order_num 1... wait, it already has paragraph 1, so active_order_num
    // is 1 and the next expected slot is 2 -- so pitch 4 should actually
    // succeed now. Let's instead prove a genuine mismatch fails:
    Pitch mismatched(5, 99, "This targets the wrong slot entirely", "sara");
    pitches.push_back(mismatched);
    auto mismatchResult = BookCore::acceptPitch(5, pitches, story, nextParagraphId);
    check("acceptPitch() fails when target_order_num doesn't match the expected slot",
          !mismatchResult.success);
    check("pitch count unchanged after failed mismatch call (no accidental mutation)",
          pitches.size() == snapshotPitchesCount + 1);

    std::cout << "\n--- Testing acceptPitch() for the next slot in sequence ---\n";
    auto secondAccept = BookCore::acceptPitch(4, pitches, story, nextParagraphId);
    check("accepting pitch 4 (targets order_num 2) succeeds now that slot 1 is filled",
          secondAccept.success);
    check("second paragraph lands at order_num 2",
          secondAccept.newParagraph.order_num == 2);
    check("story now has 2 paragraphs", story.size() == 2);

    std::cout << "\n--- Testing acceptPitchAndPersist() (disk persistence) ---\n";
    std::vector<Paragraph> persistedStory;
    std::vector<Pitch> persistedPitches = {
        Pitch(1, 1, "A persisted opening line.", "hiba")
    };
    int persistNextId = 1;
    auto persistResult = BookCore::acceptPitchAndPersist(
        1, persistedPitches, persistedStory, persistNextId, DB_FILE, PITCH_FILE);
    check("acceptPitchAndPersist() succeeds", persistResult.success);

    auto reloadedStory = Database::loadStory(DB_FILE);
    auto reloadedPitches = Database::loadPitches(PITCH_FILE);
    check("story_db.json now contains the new paragraph", reloadedStory.size() == 1);
    check("reloaded paragraph matches the in-memory one",
          reloadedStory.size() > 0 && reloadedStory[0] == persistedStory[0]);
    check("pitches_db.json reflects the Accepted status",
          reloadedPitches.size() > 0 && reloadedPitches[0].status == "Accepted");

    std::cout << "\n--- Re-checking Gatekeeper still works against BookCore output ---\n";
    Auth::logout();
    check("canModerate() false when nobody is logged in", !Gatekeeper::canModerate(story));
    Auth::login("ahmed"); // ahmed's pitch (id 4) produced the latest paragraph (order_num 2)
    check("canModerate() true: ahmed wrote the latest accepted paragraph",
          Gatekeeper::canModerate(story));
    Auth::logout();

    std::remove(DB_FILE.c_str());
    std::remove(PITCH_FILE.c_str());
    std::remove(".session.json");

    std::cout << "\n" << std::string(60, '=') << "\n";
    if (g_fail_count == 0) {
        std::cout << "ALL CHECKS PASSED (" << g_pass_count << "/" << (g_pass_count + g_fail_count) << ")\n";
    } else {
        std::cout << "SOME CHECKS FAILED (" << g_pass_count << "/" << (g_pass_count + g_fail_count)
                  << ") -- review output above\n";
    }
    std::cout << std::string(60, '=') << "\n";

    return g_fail_count == 0 ? 0 : 1;
}
