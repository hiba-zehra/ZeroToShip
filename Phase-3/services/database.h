// services/database.h
//
// Central flat-file I/O utility for story_db.json.
//
// This is the module that makes the story survive between runs: it turns
// a std::vector<Paragraph> in memory into JSON on disk (and back), reusing
// each Paragraph's own toJson()/fromJson() from Phase 1 rather than
// re-implementing serialization here.

#ifndef STORYFORGE_SERVICES_DATABASE_H
#define STORYFORGE_SERVICES_DATABASE_H

#include <vector>
#include <string>
#include "../models/Paragraph.h"
#include "../models/Pitch.h"

namespace Database {

    // Loads every paragraph from the given JSON file, in the order they
    // were saved. If the file doesn't exist yet, returns an empty vector
    // (a brand-new story with nothing written).
    std::vector<Paragraph> loadStory(const std::string& filepath = "story_db.json");

    // Persists the full list of paragraphs to the JSON file, overwriting
    // whatever was there before. Returns false on I/O failure.
    bool saveStory(const std::vector<Paragraph>& paragraphs,
                   const std::string& filepath = "story_db.json");

    // Convenience helper: loads the existing story, appends one new
    // paragraph, and saves it back in a single call.
    bool appendParagraph(const Paragraph& p,
                          const std::string& filepath = "story_db.json");

    // --- Phase 3 additions: Pitches need their own flat file too, since
    // BookCore (services/book_core.h) has to flip pitch statuses
    // (Pending -> Accepted / Rejected) and persist those changes right
    // alongside the new Paragraph they produce. Mirrors the paragraph
    // helpers above exactly, just for pitches_db.json instead.

    // Loads every pitch from the given JSON file. Missing file -> empty vector.
    std::vector<Pitch> loadPitches(const std::string& filepath = "pitches_db.json");

    // Persists the full list of pitches, overwriting the file. Returns
    // false on I/O failure.
    bool savePitches(const std::vector<Pitch>& pitches,
                      const std::string& filepath = "pitches_db.json");

    // Convenience helper: loads existing pitches, appends one new pitch,
    // and saves it back in a single call.
    bool appendPitch(const Pitch& p,
                      const std::string& filepath = "pitches_db.json");

}

#endif // STORYFORGE_SERVICES_DATABASE_H