#include "GameStateTemplate.hpp"

namespace name {

void GameStateTemplate::on_enter(sf::RenderWindow& window, entt::registry& registry) {
    // game state enter)00
}

void GameStateTemplate::handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event)  {
    // handle event))
}


void GameStateTemplate::on_exit(sf::RenderWindow& window, entt::registry& registry)  {
    // game state exit)00
}

}

ME_REGISTER_GAME_STATE("game_state_template", name::GameStateTemplate)
