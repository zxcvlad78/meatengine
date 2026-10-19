#pragma once
#include <SFML/System/Vector2.hpp>
    
namespace me::cmp {
    struct Velocity {
        sf::Vector2f linear;
        float angular;
        bool normalize = true;
    };
}