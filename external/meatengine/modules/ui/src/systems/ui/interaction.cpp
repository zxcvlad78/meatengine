#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/app/ctx/InputState.hpp>

#include <meatengine/ui/cmp/Interaction.hpp>
#include <meatengine/ui/cmp/ComputedNode.hpp>
#include <meatengine/spatial/cmp/GlobalBounds.hpp>
#include <meatengine/ui/cmp/States.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;

void run() {
    auto& r = me::MainLoop::get_registry();
    auto& in = r.ctx().get<me::app::ctx::InputState>();

    std::vector<entt::entity> candidates;
    for (auto [e, _] : r.view<me::ui::cmp::Interaction>().each())
        if (r.any_of<me::spatial::cmp::GlobalBounds>(e))
            candidates.push_back(e);

    std::sort(candidates.begin(), candidates.end(),
        [&](entt::entity a, entt::entity b) {
            uint32_t ia = r.get<me::ui::cmp::ComputedNode>(a).stack_index;
            uint32_t ib = r.get<me::ui::cmp::ComputedNode>(b).stack_index;
            return ia > ib;
        });

    entt::entity hovered = entt::null;
    for (auto e : candidates) {
        if (r.get<me::spatial::cmp::GlobalBounds>(e).rect.contains(in.mouse_pos)) {
            hovered = e;
            break;
        }
    }

    for (auto [e, i] : r.view<me::ui::cmp::Interaction>().each()) {
        auto& inter = i;
        if (e == hovered) {
            inter = in.mouse_down
                ? me::ui::cmp::Interaction::Pressed
                : me::ui::cmp::Interaction::Hovered;
        } else {
            inter = me::ui::cmp::Interaction::None;
        }
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:interaction",
    me::SystemPhase::Input,
    5,
    run
)