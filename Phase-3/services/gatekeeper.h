// services/gatekeeper.h
//
// Editor Gatekeeper.
//
// Whoever wrote the most recently accepted paragraph is the current
// "editor" -- they're the only one allowed to review pending Pitches and
// accept one. This module is the access-control check for that rule: it
// compares the active logged-in user (from services/auth.h) against the
// author of the latest paragraph in the story (the one with the highest
// order_num).

#ifndef STORYFORGE_SERVICES_GATEKEEPER_H
#define STORYFORGE_SERVICES_GATEKEEPER_H

#include <vector>
#include "../models/Paragraph.h"

namespace Gatekeeper {

    // Returns true only if:
    //   1. The story has at least one paragraph, AND
    //   2. Someone is currently logged in (Auth::getActiveUser() is not
    //      empty), AND
    //   3. That active user is the author of the paragraph with the
    //      highest order_num (i.e. the latest one).
    // Otherwise returns false -- moderation commands should be blocked.
    bool canModerate(const std::vector<Paragraph>& story);

}

#endif // STORYFORGE_SERVICES_GATEKEEPER_H