#pragma once

#include <meatengine/meatengine.hpp>
#include <godlike/godlike.hpp>

namespace godlike::states {
 
class World : public me::GameState {
public:
    void on_enter(sf::RenderWindow& window, entt::registry& registry) override {
        me::PackedEntity::spawn("testmod:player", registry);

        {auto entity = registry.create(); // tilemap
            registry.emplace<me::Transform>(entity);
            auto& tilemap = registry.emplace<me::TileMap>(entity); {
                tilemap.tileset = me::ResourceLoader::load<me::TileSet>("data/core/res/tilesets/tileset.json");
                tilemap.load_tiles("data/core/res/tilemaps/tilemap.json");
            }
        }

    }

    void handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event) override {
        
    }

    void update(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        godlike::systems::player_input(registry, window);
    }
    
    void update_deferred(sf::RenderWindow& window, entt::registry& registry, float dt) override {

    }
    
    void render(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        
    }
    void render_deferred(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        // update after "render_engine" call
    }

    void render_default_view(sf::RenderWindow& window, entt::registry& registry, float dt) {
        // render after "render_engine_default_view" call
    }

    void on_exit(sf::RenderWindow& window, entt::registry& registry) override {

	}
};

}