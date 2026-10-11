#pragma once

namespace me::ctx {
    struct InputState {
        sf::Vector2f mouse_pos;
        bool mouse_down = false;
        bool mouse_just_pressed = false;
        bool mouse_just_released = false;
    };

}