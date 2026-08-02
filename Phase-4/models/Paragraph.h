// models/Paragraph.h
//
// Defines the Paragraph class: a single accepted "block" of the story chain.
// Each Paragraph knows how to turn itself into/from a JSON object so it can
// be written to and read from a flat JSON file (story_db.json).
//
// We use nlohmann::json as our dictionary-equivalent: in Python this project
// uses to_dict()/from_dict() returning plain dicts; in C++ the natural
// equivalent is toJson()/fromJson() returning/accepting nlohmann::json
// objects, which behave like an ordered map<string, value> and serialize
// directly to a JSON file.

#ifndef STORYFORGE_MODELS_PARAGRAPH_H
#define STORYFORGE_MODELS_PARAGRAPH_H

#include <string>
#include "../json.hpp"

class Paragraph {
public:
    int id;             // unique identifier for this paragraph
    std::string text;   // the actual sentence/paragraph content
    std::string author; // username/handle of whoever wrote it
    int order_num;      // position of this paragraph in the story chain

    Paragraph() = default;

    Paragraph(int id, std::string text, std::string author, int order_num)
        : id(id), text(std::move(text)), author(std::move(author)), order_num(order_num) {}

    // Convert this Paragraph into a JSON object (the to_dict() equivalent).
    nlohmann::json toJson() const;

    // Rebuild a Paragraph from a JSON object (the from_dict() equivalent).
    static Paragraph fromJson(const nlohmann::json& data);

    bool operator==(const Paragraph& other) const {
        return id == other.id
            && text == other.text
            && author == other.author
            && order_num == other.order_num;
    }

    bool operator!=(const Paragraph& other) const {
        return !(*this == other);
    }
};

#endif // STORYFORGE_MODELS_PARAGRAPH_H