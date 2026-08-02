// services/presentation.cpp
//
// Implementation of the terminal presentation layer. See presentation.h
// for the design rationale.

#include "presentation.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace Presentation {

namespace {

    constexpr int COL_NUM    = 4;
    constexpr int COL_AUTHOR = 14;
    constexpr int COL_TARGET = 8;
    constexpr int COL_STATUS = 10;
    constexpr int COL_TEXT   = 34;

    std::string fitColumn(const std::string& s, int width) {
        if (static_cast<int>(s.size()) > width) {
            if (width <= 3) return s.substr(0, width);
            return s.substr(0, width - 3) + "...";
        }
        return s + std::string(width - static_cast<int>(s.size()), ' ');
    }

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

    void sleepMs(unsigned int ms) {
#ifdef _WIN32
        Sleep(ms);
#else
        usleep(static_cast<useconds_t>(ms) * 1000);
#endif
    }

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

}

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

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
            sleepMs(wordDelayMs);
        }
    }

    std::cout << "\n\n" << std::string(60, '-') << "\n";
    std::cout << story.size() << " paragraph(s) in the chain.\n";
}

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

}
