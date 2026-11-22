#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/hierarchy/cmp/Parent.hpp>
#include <meatengine/hierarchy/cmp/Children.hpp>
#include <meatengine/td/Math.hpp>
#include <meatengine/td/cmp/GlobalTransform3D.hpp>
#include <meatengine/td/cmp/Transform3D.hpp>

#include <functional>

namespace {

    using namespace me;

    // Recomputes GlobalTransform3D down the hierarchy, mirroring the 2D
    // hierarchy system (Update phase, priority 3).
    void walk(entt::registry& r, entt::entity e, const glm::mat4& parent_world) {
        glm::mat4 local(1.f);
        if (auto* t = r.try_get<me::td::cmp::Transform3D>(e)) {
            local = me::td::compose_transform(t->position, t->rotation, t->scale);
        }

        glm::mat4 world = parent_world * local;
        r.get_or_emplace<me::td::cmp::GlobalTransform3D>(e).matrix = world;

        if (auto* ch = r.try_get<me::hierarchy::cmp::Children>(e)) {
            for (auto c : ch->entities) {
                if (r.valid(c)) walk(r, c, world);
            }
        }
    }

    void run() {
        auto& r = me::MainLoop::get_registry();
        for (auto [e, _] : r.view<me::td::cmp::Transform3D>(
                 entt::exclude<me::hierarchy::cmp::Parent>).each()) {
            walk(r, e, glm::mat4(1.f));
        }
    }
}

ME_REGISTER_SYSTEM("engine:3d:transform",
    me::SystemPhase::Update,
    3,
    run
)
