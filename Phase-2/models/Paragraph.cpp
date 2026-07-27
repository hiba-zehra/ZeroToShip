// models/Paragraph.cpp
//
// Implementation of Paragraph's JSON serialization methods.

#include "Paragraph.h"

nlohmann::json Paragraph::toJson() const {
    return nlohmann::json{
        {"id", id},
        {"text", text},
        {"author", author},
        {"order_num", order_num}
    };
}

Paragraph Paragraph::fromJson(const nlohmann::json& data) {
    return Paragraph(
        data.at("id").get<int>(),
        data.at("text").get<std::string>(),
        data.at("author").get<std::string>(),
        data.at("order_num").get<int>()
    );
}