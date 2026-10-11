#pragma once
#include <SFML/System/Vector2.hpp>

namespace me::cmp {
    struct Transform {
        sf::Vector2f position;
        sf::Angle rotation;
        sf::Vector2f scale = {1.f, 1.f};

        Transform& operator=(const Transform& t) {
            if (this != &t) {
                position = t.position;
                rotation = t.rotation;
                scale = t.scale;
            }
            return *this;
        }

        Transform& operator=(const Transform* t) {
            if (t != nullptr && this != t) {
                position = t->position;
                rotation = t->rotation;
                scale = t->scale;
            }
            return *this;
        }
    };
}