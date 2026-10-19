#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Transform.hpp>
#include <meatengine/cmp/Sprite.hpp>
#include <meatengine/cmp/FullScreenScale.hpp>


namespace me {
    static void update() {
        auto& reg = MainLoop::get_registry();
        auto& window = MainLoop::get_window();

        auto view = reg.view<cmp::Sprite, cmp::Transform, cmp::FullScreenScale>();
        sf::Vector2f window_sizef = static_cast<sf::Vector2f>(window.getSize());

        for (auto [e, s, t, fsc] : view.each()) {
            if (!s.has_sprite()) continue;

            auto texture_size = s.sprite_ptr()->getTextureRect().size;
            if (texture_size.x == 0 || texture_size.y == 0) continue;

            sf::Vector2f target_scale = {
                window_sizef.x / static_cast<float>(texture_size.x) * fsc.multiplier.x,
                window_sizef.y / static_cast<float>(texture_size.y) * fsc.multiplier.y
            };

            t.scale = target_scale;
        }
    }
}

ME_REGISTER_SYSTEM(
    "engine:full_screen_scale",
    me::SystemPhase::Update,
    std::numeric_limits<int>::min() + 1,
    me::update
)
