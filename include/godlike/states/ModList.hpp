#pragma once
#include <meatengine/meatengine.hpp>

namespace godlike::states {

class ModList : public me::GameState {

public:
    void on_enter(sf::RenderWindow& window, entt::registry& registry) override;
    void handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event) override;
    void on_exit(sf::RenderWindow& window, entt::registry& registry) override;
};

}
