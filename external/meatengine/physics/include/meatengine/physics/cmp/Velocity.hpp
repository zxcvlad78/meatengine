#pragma once
#include <SFML/System/Vector2.hpp>
    
namespace me::physics::cmp {
    struct Velocity {
        sf::Vector2f linear;
        float angular;
        bool normalize = true;
    };
}