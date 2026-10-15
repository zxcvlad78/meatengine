#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Transform.hpp>
#include <meatengine/cmp/Parent.hpp>
#include <meatengine/cmp/Children.hpp>
#include <meatengine/cmp/GlobalTransform.hpp>
#include <meatengine/cmp/GlobalBounds.hpp>
#include <meatengine/cmp/Depth.hpp>

#include <meatengine/cmp/ui/ComputedNode.hpp>

#include <functional>
#include <algorithm>

namespace {

using namespace me;
    void walk(
        entt::registry& r,
        entt::entity e,
        const sf::Transform& parent_xf,
        int parent_depth
    ) {
        sf::Transform local;
        if (auto* t = r.try_get<me::cmp::Transform>(e)) {
            local.translate(t->position);
            local.rotate(t->rotation);
            local.scale(t->scale);
        }

        sf::Transform world = parent_xf * local;
        r.get_or_emplace<me::cmp::GlobalTransform>(e).transform = world;
        r.get_or_emplace<me::cmp::Depth>(e).value = parent_depth + 1;

        if (auto* cn = r.try_get<me::cmp::ui::ComputedNode>(e)) {
            auto p0 = world.transformPoint({0.f, 0.f});
            auto p3 = world.transformPoint(cn->size);
            sf::FloatRect aabb{
                {std::min(p0.x, p3.x), std::min(p0.y, p3.y)},
                {std::abs(p3.x - p0.x), std::abs(p3.y - p0.y)}
            };
            r.get_or_emplace<me::cmp::GlobalBounds>(e).rect = aabb;
        }

        if (auto* ch = r.try_get<me::cmp::Children>(e))
            for (auto c : ch->entities)
                if (r.valid(c)) walk(r, c, world, parent_depth + 1);
    }

    void run() {
        auto& r = me::MainLoop::get_registry();
        for (auto [e, _] : r.view<me::cmp::Transform>(entt::exclude<me::cmp::Parent>).each())
            walk(r, e, sf::Transform{}, -1);
    }
}

ME_REGISTER_SYSTEM("engine:hierarchy:update",
   me::SystemPhase::Update,
   3,
   run
)