#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/hierarchy/cmp/Children.hpp>
#include <meatengine/ui/cmp/UIRoot.hpp>
#include <meatengine/ui/cmp/ComputedNode.hpp>
#include <meatengine/abstract/cmp/ZIndex.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;

void walk(entt::registry& r, entt::entity e, uint32_t& counter) {
    r.get_or_emplace<me::ui::cmp::ComputedNode>(e).stack_index = counter++;

    auto* ch = r.try_get<me::hierarchy::cmp::Children>(e);
    if (!ch) return;

    std::vector<entt::entity> sorted = ch->entities;
    std::stable_sort(sorted.begin(), sorted.end(),
        [&](entt::entity a, entt::entity b) {
            int za = r.try_get<me::abstract::cmp::ZIndex>(a) ? r.get<me::abstract::cmp::ZIndex>(a).value : 0;
            int zb = r.try_get<me::abstract::cmp::ZIndex>(b) ? r.get<me::abstract::cmp::ZIndex>(b).value : 0;
            return za < zb;
        });

    for (auto c : sorted)
        if (r.valid(c)) walk(r, c, counter);
}

void run() {
    auto& r = me::MainLoop::get_registry();
    uint32_t counter = 0;
    for (auto [e] : r.view<me::ui::cmp::UIRoot>().each())
        walk(r, e, counter);
}

}

ME_REGISTER_SYSTEM("engine:ui:stack",
    me::SystemPhase::Update,
    4,
    run
)