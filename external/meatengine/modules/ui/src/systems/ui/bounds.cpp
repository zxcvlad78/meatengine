#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/spatial/cmp/GlobalTransform.hpp>
#include <meatengine/spatial/cmp/GlobalBounds.hpp>
#include <meatengine/ui/cmp/ComputedNode.hpp>

#include <algorithm>
#include <cmath>

namespace {

    void run() {
        auto& r = me::MainLoop::get_registry();

        for (auto [e, cn, gt] : r.view<me::ui::cmp::ComputedNode, me::spatial::cmp::GlobalTransform>().each()) {
            auto p0 = gt.transform.transformPoint({0.f, 0.f});
            auto p3 = gt.transform.transformPoint(cn.size);

            sf::FloatRect aabb{
                {std::min(p0.x, p3.x), std::min(p0.y, p3.y)},
                {std::abs(p3.x - p0.x), std::abs(p3.y - p0.y)}
            };

            r.get_or_emplace<me::spatial::cmp::GlobalBounds>(e).rect = aabb;
        }
    }

}

ME_REGISTER_SYSTEM(
    "engine:ui:bounds",
    me::SystemPhase::Update,
    4,
    run
)
