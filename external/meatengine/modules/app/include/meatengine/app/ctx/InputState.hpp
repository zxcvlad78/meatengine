#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/String.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace me::app::ctx {
    struct InputState {
        sf::Vector2f mouse_pos;
        bool mouse_down = false;
        bool mouse_just_pressed = false;
        bool mouse_just_released = false;

        // Text editing (cleared each frame in MainLoop::process_events)
        std::u32string text_entered;          // unicode code points from TextEntered
        bool backspace_pressed = false;
        bool delete_pressed = false;
        bool enter_pressed = false;
        bool left_pressed = false;
        bool right_pressed = false;
        bool up_pressed = false;
        bool down_pressed = false;
        bool home_pressed = false;
        bool end_pressed = false;
        bool ctrl_down = false;
        bool shift_down = false;
        bool select_all_pressed = false;
        bool copy_pressed = false;
        bool paste_pressed = false;
        bool cut_pressed = false;
    };

}
