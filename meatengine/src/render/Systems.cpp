#include <meatengine/render/Systems.hpp>
#include <meatengine/sprite/Systems.hpp>
#include <meatengine/ui/Systems.hpp>
#include <meatengine/tilemap/Systems.hpp>
#include <meatengine/debug_tools/common.hpp>

//RenderSystems::enabled = true;

namespace me::RenderSystems {
    bool enabled = true;

    void render(entt::registry& registry, sf::RenderWindow& window) {
        if (!enabled) return;

        me::TileMapSystems::render(registry, window);
        me::SpriteSystems::render(registry, window);
    }

    void render_default_view(entt::registry& registry, sf::RenderWindow& window) {
        if (!enabled) return;

		me::ui::Systems::render(registry, window);

    }

}