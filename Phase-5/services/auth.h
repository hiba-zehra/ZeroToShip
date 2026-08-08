// services/auth.h
//
// Identity Session Manager.
//
// Tracks which user is "active" for the terminal commands being run right
// now. There's no real login/password here -- StoryForge's users are just
// local collaborators sharing a story file, so this is a lightweight local
// state mechanism, not a security system. State is persisted to a small
// JSON session file (".session.json") so it survives across separate runs
// of the CLI in the same project folder, the same way a shell keeps you
// "logged in" until you log out.

#ifndef STORYFORGE_SERVICES_AUTH_H
#define STORYFORGE_SERVICES_AUTH_H

#include <string>

namespace Auth {

    // Logs a user in as the active identity for this session.
    // Returns false if the username is empty or the session file
    // could not be written.
    bool login(const std::string& username);

    // Clears the active session. After this, isLoggedIn() is false.
    void logout();

    // Returns the currently active username, or "" if nobody is
    // logged in / no session file exists yet.
    std::string getActiveUser();

    // True if some user is currently logged in.
    bool isLoggedIn();

}

#endif // STORYFORGE_SERVICES_AUTH_H