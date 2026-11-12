#pragma once

#include <SFML/System/Vector2.hpp>

namespace me::app::ctx {
    struct InputState {
        sf::Vector2f mouse_pos;
        bool mouse_down = false;
        bool mouse_just_pressed = false;
        bool mouse_just_released = false;
    };

}