// models/Pitch.cpp
//
// Implementation of Pitch's JSON serialization methods.

#include "Pitch.h"

nlohmann::json Pitch::toJson() const {
    return nlohmann::json{
        {"id", id},
        {"target_order_num", target_order_num},
        {"text", text},
        {"author", author},
        {"status", status}
    };
}

Pitch Pitch::fromJson(const nlohmann::json& data) {
    // .value() with a default mirrors Python's data.get("status", "Pending")
    std::string status = data.value("status", "Pending");
    return Pitch(
        data.at("id").get<int>(),
        data.at("target_order_num").get<int>(),
        data.at("text").get<std::string>(),
        data.at("author").get<std::string>(),
        status
    );
}
