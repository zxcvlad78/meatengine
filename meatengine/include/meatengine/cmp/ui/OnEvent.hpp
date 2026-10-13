#pragma once
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <SFML/System/Vector2.hpp>
#include <functional>
#include <cstdint>

namespace me::cmp::ui {
    enum class EventType : uint8_t {
        Hover, Unhover, Press, Release, Click, Focus, Blur
    };

    struct Event {
        EventType type = EventType::Click;
        sf::Vector2f local_pos{0.f, 0.f};
        entt::entity target = entt::null;
        bool handled = false;
    };

    struct OnEvent {
        std::function<void(entt::registry&, entt::entity, Event&)> cb;
    };
}