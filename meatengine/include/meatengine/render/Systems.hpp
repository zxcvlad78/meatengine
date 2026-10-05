#pragma once
#include "./Components.hpp"

#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

namespace me::RenderSystems  {
    void render(entt::registry& registry, sf::RenderWindow& window);
    void render_default_view(entt::registry& registry, sf::RenderWindow& window);
    
    extern bool enabled;
}