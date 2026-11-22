#pragma once

#include <meatengine/spatial/cmp/Transform.hpp>
#include <meatengine/physics/cmp/RectCollider.hpp>
#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <vector>

static void collision() {
    auto& reg = me::MainLoop::get_registry();
    static double total_ms = 0.0;
    static int calls = 0;
    auto t0 = std::chrono::steady_clock::now();
    float dt = me::MainLoop::get_fixed_dt();

    using Transform = me::spatial::cmp::Transform;
    using RectCollider = me::physics::cmp::RectCollider;

    for (auto [e, t, rc] : reg.view<Transform, RectCollider>().each()) {
        sf::FloatRect aabb1(t.position, rc.size);
        for (auto [e1, t1, rc1] : reg.view<Transform, RectCollider>().each()) {
            if (e1 == e) continue;
            sf::FloatRect aabb2(t1.position, rc1.size);
            if (aabb1.findIntersection(aabb2).has_value()) {
                if (rc.on_collision) rc.on_collision(e1);
                if (rc1.on_collision) rc1.on_collision(e);
            }

        }
    }
    total_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    if (calls % 60 == 0)
        std::cerr << "[phys] collision call=" << calls
                  << " avg_ms=" << total_ms / (calls ? calls : 1) << "\n";
}


ME_REGISTER_SYSTEM(
    "engine:physics:collision",
    me::SystemPhase::FixedUpdate,
    std::numeric_limits<int>::min(),
    collision
)
