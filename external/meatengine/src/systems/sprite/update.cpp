#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Sprite.hpp>
#include <meatengine/cmp/GlobalTransform.hpp>


namespace me {
    static void update() {
    auto& reg = MainLoop::get_registry();

    for (auto [e, sprite, global] : reg.view<cmp::Sprite, cmp::GlobalTransform>().each()) {
        sf::Sprite* s = sprite.sprite_ptr();
        if (!s) continue;

        if (sprite.center && sprite.offset_dirty) {
            const auto tr = s->getTextureRect();
            sprite.offset = {
                -static_cast<float>(tr.size.x) * 0.5f,
                -static_cast<float>(tr.size.y) * 0.5f
            };
            sprite.offset_dirty = false;
        }

        const sf::Vector2f pos = global.transform.transformPoint(sprite.offset);
        const auto& m = global.transform.getMatrix();
       
        const float sx = std::sqrt(m[0] * m[0] + m[1] * m[1]);
        const float sy = std::sqrt(m[4] * m[4] + m[5] * m[5]);
        const sf::Angle rot = sf::radians(std::atan2(m[1], m[0]));

        s->setPosition(pos);
        s->setRotation(rot);
        s->setScale({sx, sy});
    }
}
}

ME_REGISTER_SYSTEM(
    "engine:sprite:update",
    me::SystemPhase::Update,
    std::numeric_limits<int>::min() + 5,
    me::update
)
