// services/presentation.cpp
//
// Implementation of the terminal presentation layer. See presentation.h
// for the design rationale.

#include "presentation.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

// Some older/legacy MinGW toolchains (e.g. the classic mingw.org GCC 6.x
// distribution, as opposed to mingw-w64) never fully implemented
// std::thread / std::this_thread, since it depends on proper Win32
// threading support they didn't ship. Rather than require everyone to
// install a newer toolchain just to get a typewriter delay, we fall back
// to the native Win32 Sleep() there and keep the portable
// std::this_thread::sleep_for() path everywhere else.
#ifdef _WIN32
#include <windows.h>
#else
#include <thread>
#include <chrono>
#endif

namespace Presentation {

namespace {

    // Fixed column widths for the Pitch Registry grid. Kept as constants
    // (rather than measured per-run) so every row -- and every re-render --
    // lines up identically, which is what makes it read as a uniform grid.
    constexpr int COL_NUM    = 4;   // "#"
    constexpr int COL_AUTHOR = 14;  // "Author"
    constexpr int COL_TARGET = 8;   // "Target"
    constexpr int COL_STATUS = 10;  // "Status"
    constexpr int COL_TEXT   = 34;  // "Pitch Text"

    // Pads/truncates `s` to exactly `width` characters, left-aligned.
    // Truncation appends "..." so the reader knows content was cut off
    // rather than assuming the pitch text was actually that short.
    std::string fitColumn(const std::string& s, int width) {
        if (static_cast<int>(s.size()) > width) {
            if (width <= 3) return s.substr(0, width);
            return s.substr(0, width - 3) + "...";
        }
        return s + std::string(width - static_cast<int>(s.size()), ' ');
    }

    // Builds a horizontal rule matching the grid's column layout, e.g.
    // "+----+----------------+--------+------------+------------------------------------+"
    std::string buildRule() {
        std::ostringstream rule;
        rule << '+' << std::string(COL_NUM + 2, '-')
             << '+' << std::string(COL_AUTHOR + 2, '-')
             << '+' << std::string(COL_TARGET + 2, '-')
             << '+' << std::string(COL_STATUS + 2, '-')
             << '+' << std::string(COL_TEXT + 2, '-')
             << '+';
        return rule.str();
    }

    // Builds one bordered row: "| ... | ... | ... | ... | ... |"
    std::string buildRow(const std::string& num, const std::string& author,
                          const std::string& target, const std::string& status,
                          const std::string& text) {
        std::ostringstream row;
        row << "| " << fitColumn(num, COL_NUM)       << " "
            << "| " << fitColumn(author, COL_AUTHOR) << " "
            << "| " << fitColumn(target, COL_TARGET) << " "
            << "| " << fitColumn(status, COL_STATUS) << " "
            << "| " << fitColumn(text, COL_TEXT)      << " |";
        return row.str();
    }

} // anonymous namespace

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

// --- 1. Story Canvas Interface ------------------------------------------

void renderStoryCanvas(const std::vector<Paragraph>& story,
                        const std::string& title,
                        unsigned int wordDelayMs) {
    clearScreen();

    std::cout << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";

    if (story.empty()) {
        std::cout << "(the story is empty -- nothing has been accepted yet)\n\n";
        return;
    }

    // Join every paragraph's text into one continuous stream of words.
    // Deliberately dropping id/author/order_num here -- the canvas is
    // meant to read like a finished story, not a database dump.
    std::vector<std::string> words;
    for (const auto& paragraph : story) {
        std::istringstream lineStream(paragraph.text);
        std::string word;
        while (lineStream >> word) {
            words.push_back(word);
        }
    }

    int column = 0;
    const int wrapWidth = 60;
    for (size_t i = 0; i < words.size(); ++i) {
        const std::string& word = words[i];

        // Wrap to a new line once the current line would exceed wrapWidth,
        // so the canvas stays readable regardless of story length.
        if (column > 0 && column + 1 + static_cast<int>(word.size()) > wrapWidth) {
            std::cout << "\n";
            column = 0;
        } else if (column > 0) {
            std::cout << " ";
            ++column;
        }

        std::cout << word << std::flush;
        column += static_cast<int>(word.size());

        if (wordDelayMs > 0) {
#ifdef _WIN32
            Sleep(wordDelayMs); // Sleep() takes milliseconds directly on Windows
#else
            std::this_thread::sleep_for(std::chrono::milliseconds(wordDelayMs));
#endif
        }
    }

    std::cout << "\n\n" << std::string(60, '-') << "\n";
    std::cout << story.size() << " paragraph(s) in the chain.\n";
}

// --- 2. Pitch Registry Workspace ------------------------------------------

void renderPitchRegistry(const std::vector<Pitch>& pitches,
                          const std::string& title) {
    clearScreen();

    std::cout << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";

    const std::string rule = buildRule();

    std::cout << rule << "\n";
    std::cout << buildRow("#", "Author", "Target", "Status", "Pitch Text") << "\n";
    std::cout << rule << "\n";

    if (pitches.empty()) {
        std::cout << "| (no pitches submitted yet)"
                   << std::string(rule.size() >= 30 ? rule.size() - 30 : 0, ' ')
                   << "|\n";
        std::cout << rule << "\n";
        return;
    }

    for (size_t i = 0; i < pitches.size(); ++i) {
        const Pitch& p = pitches[i];
        std::cout << buildRow(
            std::to_string(i + 1),
            p.author,
            std::to_string(p.target_order_num),
            p.status,
            p.text
        ) << "\n";
        std::cout << rule << "\n";
    }

    std::cout << pitches.size() << " candidate pitch(es) listed above.\n";
}

} // namespace Presentation