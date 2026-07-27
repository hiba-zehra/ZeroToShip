// manual_test.cpp
//
// Phase 2 manual verification program (same spirit as Phase 1's
// manual_test.cpp -- no testing framework, just readable PASS/FAIL output).
//
// This intentionally has no menu / no input loop: Phase 2 is backend-only
// (auth, database, gatekeeper), the terminal UI itself is a later phase.

#include <iostream>
#include <cstdio>

#include "models/Paragraph.h"
#include "services/auth.h"
#include "services/database.h"
#include "services/gatekeeper.h"

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
    std::remove(DB_FILE.c_str());
    std::remove(".session.json");

    std::cout << std::string(60, '=') << "\n";
    std::cout << "StoryForge Phase 2 - Manual Verification (Auth / DB / Gatekeeper)\n";
    std::cout << std::string(60, '=') << "\n";

    std::cout << "\n--- Testing Auth ---\n";
    check("nobody logged in at startup", !Auth::isLoggedIn());
    check("getActiveUser() is empty before login", Auth::getActiveUser().empty());
    check("login('hiba') succeeds", Auth::login("hiba"));
    check("isLoggedIn() true after login", Auth::isLoggedIn());
    check("getActiveUser() returns 'hiba'", Auth::getActiveUser() == "hiba");
    check("login(\"\") is rejected", !Auth::login(""));
    Auth::logout();
    check("isLoggedIn() false after logout", !Auth::isLoggedIn());

    std::cout << "\n--- Testing Database ---\n";
    auto empty = Database::loadStory(DB_FILE);
    check("loadStory() on missing file returns empty vector", empty.empty());

    std::vector<Paragraph> story;
    story.push_back(Paragraph(1, "Once upon a time in a quiet server room...", "hiba", 1));
    story.push_back(Paragraph(2, "...a lone process refused to terminate.", "ahmed", 2));
    check("saveStory() succeeds", Database::saveStory(story, DB_FILE));

    auto reloaded = Database::loadStory(DB_FILE);
    check("loadStory() returns correct number of paragraphs", reloaded.size() == 2);
    check("round-tripped paragraph 1 matches original", reloaded.size() > 0 && reloaded[0] == story[0]);
    check("round-tripped paragraph 2 matches original", reloaded.size() > 1 && reloaded[1] == story[1]);

    Paragraph p3(3, "The logs, however, told a different story.", "hiba", 3);
    check("appendParagraph() succeeds", Database::appendParagraph(p3, DB_FILE));
    auto afterAppend = Database::loadStory(DB_FILE);
    check("story has 3 paragraphs after append", afterAppend.size() == 3);

    std::cout << "\n--- Testing Gatekeeper ---\n";
    Auth::logout();
    check("canModerate() false when nobody is logged in", !Gatekeeper::canModerate(afterAppend));

    Auth::login("ahmed");
    check("canModerate() false for a user who isn't the latest author",
          !Gatekeeper::canModerate(afterAppend));

    Auth::login("hiba");
    check("canModerate() true for the latest paragraph's author",
          Gatekeeper::canModerate(afterAppend));

    std::vector<Paragraph> noStory;
    check("canModerate() false on an empty story", !Gatekeeper::canModerate(noStory));

    Auth::logout();
    std::remove(DB_FILE.c_str());
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