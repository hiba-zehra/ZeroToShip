// services/auth.cpp
//
// Implementation of the Identity Session Manager. Uses nlohmann::json for
// the session file, same as the rest of the project, instead of inventing
// a second ad-hoc file format.

#include "auth.h"
#include <fstream>
#include "../json.hpp"
using json = nlohmann::json;

namespace {
    // Local file tracking who's "logged in" right now. Lives alongside
    // story_db.json in whatever folder the CLI is run from.
    const std::string SESSION_FILE = ".session.json";
}

namespace Auth {

bool login(const std::string& username) {
    if (username.empty()) return false;

    json session;
    session["active_user"] = username;

    std::ofstream out(SESSION_FILE, std::ios::trunc);
    if (!out.is_open()) return false;
    out << session.dump(2);
    return true;
}

void logout() {
    json session;
    session["active_user"] = "";

    std::ofstream out(SESSION_FILE, std::ios::trunc);
    if (out.is_open()) {
        out << session.dump(2);
    }
}

std::string getActiveUser() {
    std::ifstream in(SESSION_FILE);
    if (!in.is_open()) return ""; // no session yet -> nobody logged in

    json session;
    try {
        in >> session;
    } catch (const json::parse_error&) {
        return ""; // corrupt/empty session file -> treat as logged out
    }

    return session.value("active_user", "");
}

bool isLoggedIn() {
    return !getActiveUser().empty();
}

}