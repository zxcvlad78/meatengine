#pragma once
#include <meatengine/meatengine.hpp>

class GameStateTemplate : public me::GameState {
    public:
    void on_enter(sf::RenderWindow& window, entt::registry& registry) override {
        // game state enter))
    }

    void handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event) override {
        // handle event))
    }

    void update(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        // update before "update_engine" call
    }

    void update_deferred(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        // update after "update_engine" call
    }

    void render(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        // render before "render_engine" call
    }
    void render_deferred(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        // render after "render_engine" call
    }

    void render_default_view(sf::RenderWindow& window, entt::registry& registry, float dt) {
        // render after "render_engine_default_view" call
    }

    void on_exit(sf::RenderWindow& window, entt::registry& registry) override {
        // game state exit)00
    }
};

