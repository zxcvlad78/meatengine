#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <meatengine/ctx/InputState.hpp>

#include <meatengine/cmp/ui/Interaction.hpp>
#include <meatengine/cmp/ui/ComputedNode.hpp>
#include <meatengine/cmp/GlobalBounds.hpp>
#include <meatengine/cmp/ui/States.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;

void run() {
    auto& r = me::MainLoop::get_registry();
    auto& in = r.ctx().get<me::ctx::InputState>();

    std::vector<entt::entity> candidates;
    for (auto [e, _] : r.view<me::cmp::ui::Interaction>().each())
        if (r.any_of<me::cmp::GlobalBounds>(e))
            candidates.push_back(e);

    std::sort(candidates.begin(), candidates.end(),
        [&](entt::entity a, entt::entity b) {
            uint32_t ia = r.get<me::cmp::ui::ComputedNode>(a).stack_index;
            uint32_t ib = r.get<me::cmp::ui::ComputedNode>(b).stack_index;
            return ia > ib;
        });

    entt::entity hovered = entt::null;
    for (auto e : candidates) {
        if (r.get<me::cmp::GlobalBounds>(e).rect.contains(in.mouse_pos)) {
            hovered = e;
            break;
        }
    }

    for (auto [e, i] : r.view<me::cmp::ui::Interaction>().each()) {
        auto& inter = i;
        if (e == hovered) {
            inter = in.mouse_down
                ? me::cmp::ui::Interaction::Pressed
                : me::cmp::ui::Interaction::Hovered;
        } else {
            inter = me::cmp::ui::Interaction::None;
        }
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:interaction",
    me::SystemPhase::Input,
    5,
    run
)