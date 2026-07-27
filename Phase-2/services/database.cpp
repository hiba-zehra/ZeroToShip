// services/database.cpp
//
// Implementation of the flat-file I/O utility. story_db.json is stored as
// a plain JSON array of Paragraph objects, exactly the shape produced by
// Paragraph::toJson() in Phase 1.

#include "database.h"
#include <fstream>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

namespace Database {

std::vector<Paragraph> loadStory(const std::string& filepath) {
    std::vector<Paragraph> story;

    std::ifstream in(filepath);
    if (!in.is_open()) {
        return story; // no file yet -> treat as an empty, brand-new story
    }

    json data;
    try {
        in >> data;
    } catch (const json::parse_error& e) {
        std::cerr << "Database: " << filepath << " is not valid JSON ("
                  << e.what() << "). Treating story as empty.\n";
        return story;
    }

    if (!data.is_array()) {
        std::cerr << "Database: " << filepath << " did not contain a JSON array.\n";
        return story;
    }

    story.reserve(data.size());
    for (const auto& entry : data) {
        story.push_back(Paragraph::fromJson(entry));
    }
    return story;
}

bool saveStory(const std::vector<Paragraph>& paragraphs, const std::string& filepath) {
    json data = json::array();
    for (const auto& p : paragraphs) {
        data.push_back(p.toJson());
    }

    std::ofstream out(filepath, std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Database: could not open " << filepath << " for writing.\n";
        return false;
    }

    out << data.dump(2); // pretty-printed, matches manual_test.cpp's dump() usage
    return true;
}

bool appendParagraph(const Paragraph& p, const std::string& filepath) {
    std::vector<Paragraph> story = loadStory(filepath);
    story.push_back(p);
    return saveStory(story, filepath);
}

}