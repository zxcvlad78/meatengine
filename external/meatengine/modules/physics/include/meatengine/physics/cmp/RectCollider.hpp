#pragma once

#include <entt/entity/entity.hpp>
#include <SFML/System/Vector2.hpp>
#include <functional>    

namespace me::physics::cmp {
    struct RectCollider {
        sf::Vector2f size;
        std::function<void(entt::entity e)> on_collision;
    };
}