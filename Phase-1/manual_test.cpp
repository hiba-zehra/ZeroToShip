// manual_test.cpp
//
// Phase 1 manual verification program.
//
// This is NOT an automated unit-test framework (no Catch2/GoogleTest) --
// it's a simple, readable program a student runs by hand to eyeball whether
// the Paragraph and Pitch models correctly:
//     1. Store the attributes they were given.
//     2. Convert to JSON with toJson().
//     3. Rebuild an equivalent object from that JSON with fromJson().
//     4. Survive a full round-trip through json::dump()/json::parse(),
//        simulating what will actually happen with story_db.json later.
//
// Build & run (see README / Makefile):
//     g++ -std=c++17 -Imodels manual_test.cpp models/Paragraph.cpp models/Pitch.cpp -o manual_test
//     ./manual_test

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "models/Paragraph.h"
#include "models/Pitch.h"

using json = nlohmann::json;

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

bool testParagraph() {
    std::cout << "\n--- Testing Paragraph ---\n";
    bool all_ok = true;

    Paragraph p(1, "Once upon a time, in a quiet village...", "alice", 1);
    std::cout << "Created: id=" << p.id << " author=" << p.author
              << " order_num=" << p.order_num << " text=\"" << p.text << "\"\n";

    all_ok &= check("attributes stored correctly",
        p.id == 1
        && p.text == "Once upon a time, in a quiet village..."
        && p.author == "alice"
        && p.order_num == 1);

    // toJson()
    json p_json = p.toJson();
    std::cout << "toJson() -> " << p_json.dump() << "\n";
    json expected = {
        {"id", 1},
        {"text", "Once upon a time, in a quiet village..."},
        {"author", "alice"},
        {"order_num", 1}
    };
    all_ok &= check("toJson() matches expected structure", p_json == expected);

    // fromJson()
    Paragraph p_rebuilt = Paragraph::fromJson(p_json);
    all_ok &= check("fromJson() reconstructs an equal object", p_rebuilt == p);

    // Full JSON round trip (simulates writing/reading story_db.json)
    std::string blob = p_json.dump();
    Paragraph p_from_blob = Paragraph::fromJson(json::parse(blob));
    all_ok &= check("survives full JSON round trip", p_from_blob == p);

    return all_ok;
}

bool testPitch() {
    std::cout << "\n--- Testing Pitch ---\n";
    bool all_ok = true;

    Pitch pitch(101, 2, "Suddenly, a stranger knocked on the door.", "bob", "Pending");
    std::cout << "Created: id=" << pitch.id << " target_order_num=" << pitch.target_order_num
              << " author=" << pitch.author << " status=" << pitch.status
              << " text=\"" << pitch.text << "\"\n";

    all_ok &= check("attributes stored correctly",
        pitch.id == 101
        && pitch.target_order_num == 2
        && pitch.text == "Suddenly, a stranger knocked on the door."
        && pitch.author == "bob"
        && pitch.status == "Pending");

    // toJson()
    json pitch_json = pitch.toJson();
    std::cout << "toJson() -> " << pitch_json.dump() << "\n";
    json expected = {
        {"id", 101},
        {"target_order_num", 2},
        {"text", "Suddenly, a stranger knocked on the door."},
        {"author", "bob"},
        {"status", "Pending"}
    };
    all_ok &= check("toJson() matches expected structure", pitch_json == expected);

    // fromJson()
    Pitch pitch_rebuilt = Pitch::fromJson(pitch_json);
    all_ok &= check("fromJson() reconstructs an equal object", pitch_rebuilt == pitch);

    // Full JSON round trip
    std::string blob = pitch_json.dump();
    Pitch pitch_from_blob = Pitch::fromJson(json::parse(blob));
    all_ok &= check("survives full JSON round trip", pitch_from_blob == pitch);

    // Status transition sanity check (Accepted/Rejected are valid too)
    pitch.setStatus("Accepted");
    all_ok &= check("status can be updated to Accepted", pitch.toJson()["status"] == "Accepted");

    // Invalid status should throw, proving validation works
    try {
        Pitch bad(999, 1, "bad status test", "eve", "Maybe");
        all_ok &= check("invalid status correctly rejected", false);
    } catch (const std::invalid_argument& e) {
        all_ok &= check(std::string("invalid status correctly rejected (") + e.what() + ")", true);
    }

    return all_ok;
}

int main() {
    std::cout << std::string(60, '=') << "\n";
    std::cout << "StoryForge Phase 1 - Manual Model Verification (C++)\n";
    std::cout << std::string(60, '=') << "\n";

    bool paragraph_ok = testParagraph();
    bool pitch_ok = testPitch();

    std::cout << "\n" << std::string(60, '=') << "\n";
    if (paragraph_ok && pitch_ok) {
        std::cout << "ALL CHECKS PASSED (" << g_pass_count << "/" << (g_pass_count + g_fail_count) << ")\n";
    } else {
        std::cout << "SOME CHECKS FAILED (" << g_pass_count << "/" << (g_pass_count + g_fail_count)
                  << ") -- review output above\n";
    }
    std::cout << std::string(60, '=') << "\n";

    return (paragraph_ok && pitch_ok) ? 0 : 1;
}
