// services/book_core.h
//
// The Book Core — this is the actual "engine" of StoryForge. Everything
// before this phase (models, auth, database, gatekeeper) was just plumbing
// to store data and check permissions. This is where the story actually
// *moves forward*.
//
// Two jobs live here:
//
//   1. Chronological Sorting Engine
//      Paragraphs get appended to the story in whatever order they were
//      accepted, but nothing guarantees they're stored in order_num order.
//      getChronologicalOrder() hands back the story sorted the way a
//      reader should see it, using a lambda comparator on order_num --
//      the C++ equivalent of Python's `sorted(paragraphs, key=lambda x:
//      x.order_num)`.
//
//   2. State Transition Pipeline ("accept a pitch")
//      When the current editor (see services/gatekeeper.h) accepts a
//      Pitch, three things need to happen together:
//        a. A brand-new Paragraph is created at active_order_num + 1
//           (active_order_num = the highest order_num currently in the
//           story, or 0 if the story is empty).
//        b. That Pitch's status flips from "Pending" to "Accepted".
//        c. Every *other* Pending pitch competing for the same
//           target_order_num flips to "Rejected" -- only one pitch per
//           position can ever win.
//      This has to be atomic: either all three happen, or none of them
//      do. We never want a story where a paragraph exists but the pitch
//      that produced it is still sitting there "Pending", or where two
//      pitches for the same slot both ended up "Accepted".

#ifndef STORYFORGE_SERVICES_BOOK_CORE_H
#define STORYFORGE_SERVICES_BOOK_CORE_H

#include <string>
#include <vector>
#include "../models/Paragraph.h"
#include "../models/Pitch.h"

namespace BookCore {

    // --- 1. Chronological Sorting Engine ---------------------------------

    // Returns a *copy* of `paragraphs` sorted into reading order (ascending
    // order_num), using a lambda key exactly like Python's
    // `sorted(paragraphs, key=lambda x: x.order_num)`.
    std::vector<Paragraph> getChronologicalOrder(std::vector<Paragraph> paragraphs);

    // The highest order_num currently in the story, i.e. the position of
    // the most recently accepted paragraph. Returns 0 for an empty story
    // (so the *first* accepted paragraph lands at order_num 1).
    int getActiveOrderNum(const std::vector<Paragraph>& story);


    // --- 2. State Transition Pipeline ("accept a pitch") -----------------

    // Why we can't just return a Paragraph: acceptPitch can fail (bad
    // pitch id, pitch already resolved, mismatched target_order_num) and
    // callers need to know *why* without a Paragraph's default
    // constructor pretending everything worked. success == false means
    // nothing in `pitches` or `story` was modified at all.
    struct AcceptResult {
        bool success = false;
        std::string message;                  // human-readable reason (esp. on failure)
        Paragraph newParagraph;                // only meaningful if success == true
        std::vector<int> rejectedPitchIds;     // ids of pitches flipped to "Rejected"
    };

    // The atomic pipeline itself.
    //
    //   pitchId        - id of the Pitch being accepted
    //   pitches         - the full in-memory pitch list (mutated in place
    //                     on success: winner -> "Accepted", every other
    //                     Pending pitch for the same slot -> "Rejected")
    //   story           - the full in-memory paragraph list (a new
    //                     Paragraph is appended on success)
    //   nextParagraphId - id to assign to the new Paragraph; the caller
    //                     owns id generation, so this is passed by
    //                     reference and incremented only on success
    //
    // All validation happens *before* any mutation, so a failed call
    // leaves `pitches`, `story`, and `nextParagraphId` completely
    // untouched -- that's what makes this "atomic".
    AcceptResult acceptPitch(int pitchId,
                              std::vector<Pitch>& pitches,
                              std::vector<Paragraph>& story,
                              int& nextParagraphId);

    // Convenience wrapper: does everything acceptPitch() does, and if (and
    // only if) it succeeds, also persists both the updated pitch list and
    // the updated story to disk via services/database.h. If either save
    // fails, the in-memory changes are rolled back so we never end up with
    // memory and disk disagreeing.
    AcceptResult acceptPitchAndPersist(int pitchId,
                                        std::vector<Pitch>& pitches,
                                        std::vector<Paragraph>& story,
                                        int& nextParagraphId,
                                        const std::string& storyFilepath = "story_db.json",
                                        const std::string& pitchesFilepath = "pitches_db.json");

}

#endif // STORYFORGE_SERVICES_BOOK_CORE_H
