#pragma once
#include <SFML/System/Vector2.hpp>

namespace me::cmp {
    struct Offset {
        sf::Vector2f position;

        void center(sf::Vector2f rect_size) {
            position = {
                -static_cast<float>(rect_size.x) / 2.f,
                -static_cast<float>(rect_size.y) / 2.f
            };
        }
    };
}