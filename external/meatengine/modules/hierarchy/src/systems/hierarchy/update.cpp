#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/spatial/cmp/Transform.hpp>
#include <meatengine/hierarchy/cmp/Parent.hpp>
#include <meatengine/hierarchy/cmp/Children.hpp>
#include <meatengine/spatial/cmp/GlobalTransform.hpp>
#include <meatengine/spatial/cmp/GlobalBounds.hpp>
#include <meatengine/abstract/cmp/Depth.hpp>

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
        if (auto* t = r.try_get<me::spatial::cmp::Transform>(e)) {
            local.translate(t->position);
            local.rotate(t->rotation);
            local.scale(t->scale);
        }

        sf::Transform world = parent_xf * local;
        r.get_or_emplace<me::spatial::cmp::GlobalTransform>(e).transform = world;
        r.get_or_emplace<me::abstract::cmp::Depth>(e).value = parent_depth + 1;

        if (auto* ch = r.try_get<me::hierarchy::cmp::Children>(e))
            for (auto c : ch->entities)
                if (r.valid(c)) walk(r, c, world, parent_depth + 1);
    }

    void run() {
        auto& r = me::MainLoop::get_registry();
        for (auto [e, _] : r.view<me::spatial::cmp::Transform>(entt::exclude<me::hierarchy::cmp::Parent>).each())
            walk(r, e, sf::Transform{}, -1);
    }
}

ME_REGISTER_SYSTEM("engine:hierarchy:update",
   me::SystemPhase::Update,
   3,
   run
)