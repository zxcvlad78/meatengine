#pragma once

#include <meatengine/spatial/cmp/Transform.hpp>
#include <meatengine/physics/cmp/RectCollider.hpp>
#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <vector>

static void collision() {
    auto& reg = me::MainLoop::get_registry();
    float dt = me::MainLoop::get_fixed_dt();

    using Transform = me::spatial::cmp::Transform;
    using RectCollider = me::physics::cmp::RectCollider;

    for (auto [e, t, rc] : reg.view<Transform, RectCollider>().each()) {
        sf::FloatRect aabb1(t.position, rc.size);
        for (auto [e1, t1, rc1] : reg.view<Transform, RectCollider>().each()) {
            if (e1 == e) continue;
            sf::FloatRect aabb2(t.position, rc.size);
            if (aabb1.findIntersection(aabb2).has_value()) {
                rc.on_collision(e1);
                rc1.on_collision(e);
            }
        
        }
    }
}


ME_REGISTER_SYSTEM(
    "engine:physics:collision",
    me::SystemPhase::FixedUpdate,
    std::numeric_limits<int>::min(),
    collision
)
