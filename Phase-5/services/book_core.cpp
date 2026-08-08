// services/book_core.cpp
//
// Implementation of the Chronological Sorting Engine and the
// accept-a-pitch state transition pipeline.

#include "book_core.h"
#include "database.h"
#include <algorithm>

namespace BookCore {

std::vector<Paragraph> getChronologicalOrder(std::vector<Paragraph> paragraphs) {
    // This lambda is the direct C++ equivalent of Python's
    // `sorted(paragraphs, key=lambda x: x.order_num)`: it just compares
    // two paragraphs by their order_num field.
    std::sort(paragraphs.begin(), paragraphs.end(),
              [](const Paragraph& a, const Paragraph& b) {
                  return a.order_num < b.order_num;
              });
    return paragraphs;
}

int getActiveOrderNum(const std::vector<Paragraph>& story) {
    if (story.empty()) {
        return 0; // nothing accepted yet -> the next paragraph starts at 1
    }

    const Paragraph& latest = *std::max_element(
        story.begin(), story.end(),
        [](const Paragraph& a, const Paragraph& b) {
            return a.order_num < b.order_num;
        });
    return latest.order_num;
}

AcceptResult acceptPitch(int pitchId,
                          std::vector<Pitch>& pitches,
                          std::vector<Paragraph>& story,
                          int& nextParagraphId) {
    AcceptResult result;

    // --- Phase A: validate everything first. Nothing gets mutated until
    // we're sure the whole pipeline can succeed -- that's what keeps this
    // "atomic".

    auto it = std::find_if(pitches.begin(), pitches.end(),
                            [pitchId](const Pitch& p) { return p.id == pitchId; });

    if (it == pitches.end()) {
        result.success = false;
        result.message = "No pitch found with id " + std::to_string(pitchId) + ".";
        return result;
    }

    if (it->status != "Pending") {
        result.success = false;
        result.message = "Pitch " + std::to_string(pitchId) +
                          " has already been resolved (status: " + it->status + ").";
        return result;
    }

    int activeOrderNum = getActiveOrderNum(story);
    int newOrderNum = activeOrderNum + 1;

    if (it->target_order_num != newOrderNum) {
        result.success = false;
        result.message = "Pitch " + std::to_string(pitchId) + " targets order_num " +
                          std::to_string(it->target_order_num) + ", but the story is " +
                          "currently expecting order_num " + std::to_string(newOrderNum) + ".";
        return result;
    }

    // --- Phase B: everything checks out, so now (and only now) do we
    // actually mutate state.

    Pitch& winner = *it;
    int targetOrderNum = winner.target_order_num;

    // b. Flip the winning pitch to Accepted.
    winner.setStatus("Accepted");

    // c. Flip every other Pending competitor for the same slot to Rejected.
    for (Pitch& p : pitches) {
        if (p.id != pitchId && p.target_order_num == targetOrderNum && p.status == "Pending") {
            p.setStatus("Rejected");
            result.rejectedPitchIds.push_back(p.id);
        }
    }

    // a. Instantiate the new Paragraph at active_order_num + 1.
    Paragraph newParagraph(nextParagraphId, winner.text, winner.author, newOrderNum);
    story.push_back(newParagraph);
    ++nextParagraphId;

    result.success = true;
    result.message = "Pitch " + std::to_string(pitchId) + " accepted; paragraph " +
                      std::to_string(newParagraph.id) + " created at order_num " +
                      std::to_string(newOrderNum) + ".";
    result.newParagraph = newParagraph;
    return result;
}

AcceptResult acceptPitchAndPersist(int pitchId,
                                    std::vector<Pitch>& pitches,
                                    std::vector<Paragraph>& story,
                                    int& nextParagraphId,
                                    const std::string& storyFilepath,
                                    const std::string& pitchesFilepath) {
    // Snapshot everything so we can roll back cleanly if a save fails
    // partway through -- we never want memory and disk to disagree.
    std::vector<Pitch> pitchesBefore = pitches;
    std::vector<Paragraph> storyBefore = story;
    int nextIdBefore = nextParagraphId;

    AcceptResult result = acceptPitch(pitchId, pitches, story, nextParagraphId);
    if (!result.success) {
        return result; // acceptPitch() already guarantees nothing was mutated
    }

    bool storySaved = Database::saveStory(story, storyFilepath);
    bool pitchesSaved = storySaved && Database::savePitches(pitches, pitchesFilepath);

    if (!storySaved || !pitchesSaved) {
        // Roll back the in-memory mutation so callers never see a
        // "success" state that doesn't match what's on disk.
        pitches = pitchesBefore;
        story = storyBefore;
        nextParagraphId = nextIdBefore;

        result.success = false;
        result.message = "Pitch " + std::to_string(pitchId) +
                          " was validated but could not be persisted to disk; "
                          "all changes were rolled back.";
        result.rejectedPitchIds.clear();
        result.newParagraph = Paragraph();
    }

    return result;
}

}
