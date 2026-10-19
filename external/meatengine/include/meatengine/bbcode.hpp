#pragma once

#include <string>
#include <SFML/Graphics.hpp>

namespace me::BBCode {
    struct TextFragment {
        std::string text;
        sf::Color color;
        unsigned int character_size;
        bool bold;
        bool italic;
        bool underlined;

        TextFragment(const std::string& txt = "",
            sf::Color col = sf::Color::White,
            unsigned int size = 16,
            bool b = false,
            bool i = false,
            bool u = false
        ) : text(txt), color(col), character_size(size), bold(b), italic(i), underlined(u) {}
    };

    std::vector<TextFragment> parse(const std::string& input, unsigned int default_size = 16);
} // namespace me::BBCode
