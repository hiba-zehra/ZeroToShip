// models/Pitch.h
//
// Defines the Pitch class: a proposed continuation of the story submitted by
// some author, targeting a specific order_num in the chain. The current
// "editor" (author of the latest accepted paragraph) reviews pending pitches
// and marks exactly one as "Accepted" (the rest become "Rejected").

#ifndef STORYFORGE_MODELS_PITCH_H
#define STORYFORGE_MODELS_PITCH_H

#include <string>
#include <stdexcept>
#include <array>
#include <algorithm>
#include "../json.hpp"

class Pitch {
public:
static const std::array<const char*, 3> VALID_STATUSES;

    int id;                  // unique identifier for this pitch
    int target_order_num;    // order_num this pitch is competing for
    std::string text;        // the proposed paragraph text
    std::string author;      // username/handle of the pitch's author
    std::string status;      // "Pending" | "Accepted" | "Rejected"

    Pitch() = default;

    Pitch(int id, int target_order_num, std::string text, std::string author,
          std::string status = "Pending")
        : id(id), target_order_num(target_order_num), text(std::move(text)),
          author(std::move(author)), status(std::move(status)) {
        validateStatus(this->status);
    }

    // Throws std::invalid_argument if the status is not one of VALID_STATUSES.
    static void validateStatus(const std::string& candidate) {
        bool ok = std::any_of(VALID_STATUSES.begin(), VALID_STATUSES.end(),
                               [&](const char* s) { return candidate == s; });
        if (!ok) {
            throw std::invalid_argument("Invalid status '" + candidate +
                                         "'. Must be one of Pending/Accepted/Rejected.");
        }
    }

    void setStatus(const std::string& newStatus) {
        validateStatus(newStatus);
        status = newStatus;
    }

    // Convert this Pitch into a JSON object (the to_dict() equivalent).
    nlohmann::json toJson() const;

    // Rebuild a Pitch from a JSON object (the from_dict() equivalent).
    static Pitch fromJson(const nlohmann::json& data);

    bool operator==(const Pitch& other) const {
        return id == other.id
            && target_order_num == other.target_order_num
            && text == other.text
            && author == other.author
            && status == other.status;
    }

    bool operator!=(const Pitch& other) const {
        return !(*this == other);
    }
};

#endif // STORYFORGE_MODELS_PITCH_H