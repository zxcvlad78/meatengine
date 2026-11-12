#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <entt/resource/resource.hpp>

#include <meatengine/assets/Resources.hpp>
#include <meatengine/assets/ResourceLoader.hpp>

namespace me::ui::cmp {
    struct FillRect {
        sf::RectangleShape shape;

        bool background = false;
        bool styled = true;
        bool dirty = true;

        FillRect() = default;

        explicit FillRect(sf::Vector2f size) {
            shape.setSize(size);
            shape.setFillColor(sf::Color::White);
            shape.setOutlineColor(sf::Color::Black);
            shape.setOutlineThickness(0.f);
        }
    };

}