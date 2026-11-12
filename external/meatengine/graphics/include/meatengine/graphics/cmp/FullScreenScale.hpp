#pragma once
#include <SFML/System/Vector2.hpp>

namespace me::graphics::cmp {
    struct FullScreenScale {
        sf::Vector2f multiplier = {1.f, 1.f};
    };
}
