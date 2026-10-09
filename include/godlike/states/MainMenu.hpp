#pragma once
#include <meatengine/meatengine.hpp>

namespace godlike::states {

namespace tags {
    struct BGFillRect {};
    struct CButton {};
}

class MainMenu : public me::GameState {

public:
    void on_enter(sf::RenderWindow& window, entt::registry& registry) override;
    void handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event) override;
    void update(sf::RenderWindow& window, entt::registry& registry, float dt) override;
    void update_deferred(sf::RenderWindow& window, entt::registry& registry, float dt);
    void render(sf::RenderWindow& window, entt::registry& registry, float dt) override;
    void render_deferred(sf::RenderWindow& window, entt::registry& registry, float dt);
    void render_default_view(sf::RenderWindow& window, entt::registry& registry, float dt);
    void on_exit(sf::RenderWindow& window, entt::registry& registry) override;
};

}
