// services/gatekeeper.cpp
//
// Implementation of the editor access-control check.

#include "gatekeeper.h"
#include "auth.h"
#include <algorithm>

namespace Gatekeeper {

bool canModerate(const std::vector<Paragraph>& story) {
    if (story.empty()) return false; // nothing written yet -> no editor exists

    std::string activeUser = Auth::getActiveUser();
    if (activeUser.empty()) return false; // nobody logged in

    // The "latest" paragraph is the one with the highest order_num,
    // not necessarily the last element in the vector (callers might not
    // guarantee ordering), so we search for it explicitly.
    const Paragraph& latest = *std::max_element(
        story.begin(), story.end(),
        [](const Paragraph& a, const Paragraph& b) {
            return a.order_num < b.order_num;
        });

    return activeUser == latest.author;
}

}