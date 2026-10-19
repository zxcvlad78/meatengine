#pragma once
#include <functional>
#include <entt/entt.hpp>

namespace me::cmp {
    struct Timer {
        float wait_time = 1.f;
        std::function<void(entt::registry&)> timeout_func; 
        
        float time_left = wait_time;

        bool one_shot = true;
        bool paused = true;
        
        void start() {
            paused = false;
            time_left = wait_time;
        }

    };
}