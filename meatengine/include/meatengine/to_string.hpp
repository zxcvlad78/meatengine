#pragma once
#include <SFML/Graphics.hpp>

namespace sf {
    inline std::string to_string(const sf::Color& color) {
        return "Color(" + std::to_string(color.r) + ", "
        + std::to_string(color.g) + ", "
        + std::to_string(color.b) + ", "
        + std::to_string(color.a) + ")";
    }
}