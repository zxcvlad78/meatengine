#pragma once
#include <SFML/System/Vector2.hpp>
#include <cstdint>

namespace me::cmp::ui {
    struct ComputedNode {
        sf::Vector2f size{0.f, 0.f};
        sf::Vector2f content_size{0.f, 0.f};
        uint32_t stack_index = 0;
    };
}