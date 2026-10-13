#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Children.hpp>
#include <meatengine/cmp/ui/UIRoot.hpp>
#include <meatengine/cmp/ui/ComputedNode.hpp>
#include <meatengine/cmp/ZIndex.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;

void walk(entt::registry& r, entt::entity e, uint32_t& counter) {
    r.get_or_emplace<me::cmp::ui::ComputedNode>(e).stack_index = counter++;

    auto* ch = r.try_get<me::cmp::Children>(e);
    if (!ch) return;

    std::vector<entt::entity> sorted = ch->entities;
    std::stable_sort(sorted.begin(), sorted.end(),
        [&](entt::entity a, entt::entity b) {
            int za = r.try_get<me::cmp::ZIndex>(a) ? r.get<me::cmp::ZIndex>(a).value : 0;
            int zb = r.try_get<me::cmp::ZIndex>(b) ? r.get<me::cmp::ZIndex>(b).value : 0;
            return za < zb;
        });

    for (auto c : sorted)
        if (r.valid(c)) walk(r, c, counter);
}

void run() {
    auto& r = me::MainLoop::get_registry();
    uint32_t counter = 0;
    for (auto [e] : r.view<me::cmp::ui::UIRoot>().each())
        walk(r, e, counter);
}

}

ME_REGISTER_SYSTEM("engine:ui:stack",
    me::SystemPhase::Update,
    4,
    run
)