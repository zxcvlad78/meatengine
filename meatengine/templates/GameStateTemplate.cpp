#include "GameStateTemplate.hpp"

namespace name {

void GameStateTemplate::on_enter(sf::RenderWindow& window, entt::registry& registry) {
    // game state enter)00
}

void GameStateTemplate::handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event)  {
    // handle event))
}

void GameStateTemplate::update(sf::RenderWindow& window, entt::registry& registry, float dt)  {
    // update before "update_engine" call
}

void GameStateTemplate::update_deferred(sf::RenderWindow& window, entt::registry& registry, float dt)  {
    // update after "update_engine" call
}

void GameStateTemplate::render(sf::RenderWindow& window, entt::registry& registry, float dt)  {
    // render before "render_engine" call
}
void GameStateTemplate::render_deferred(sf::RenderWindow& window, entt::registry& registry, float dt)  {
    // render after "render_engine" call
}

void GameStateTemplate::render_default_view(sf::RenderWindow& window, entt::registry& registry, float dt) {
    // render after "render_engine_default_view" call
}

void GameStateTemplate::on_exit(sf::RenderWindow& window, entt::registry& registry)  {
    // game state exit)00
}

}

ME_REGISTER_GAME_STATE("game_state_template", name::GameStateTemplate)
