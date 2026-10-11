#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

namespace me {

class GameState {
public:
    virtual ~GameState() = default;

    virtual void on_enter(sf::RenderWindow& window, entt::registry& registry) = 0;
    virtual void handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event) = 0;
    virtual void on_exit(sf::RenderWindow& window, entt::registry& registry) = 0;
};

} // namespace me
