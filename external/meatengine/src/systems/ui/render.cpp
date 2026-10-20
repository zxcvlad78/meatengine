#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/ui/ComputedNode.hpp>
#include <meatengine/cmp/GlobalTransform.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/Label.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;

void run() {
    auto& r = me::MainLoop::get_registry();
    auto& win = me::MainLoop::get_window();

    std::vector<entt::entity> order;
    for (auto [e, _] : r.view<me::cmp::ui::ComputedNode>().each())
        order.push_back(e);

    std::sort(order.begin(), order.end(), [&](entt::entity a, entt::entity b) {
        return r.get<me::cmp::ui::ComputedNode>(a).stack_index
            < r.get<me::cmp::ui::ComputedNode>(b).stack_index;
    });

    for (auto e : order) {
        sf::Transform xf = r.try_get<me::cmp::GlobalTransform>(e)
                        ? r.get<me::cmp::GlobalTransform>(e).transform
                        : sf::Transform{};

        if (auto* fr = r.try_get<me::cmp::ui::FillRect>(e))
            win.draw(fr->shape, xf);

        if (auto* lb = r.try_get<me::cmp::ui::Label>(e); lb && lb->text)
            win.draw(*lb->text, xf);
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:render",
    me::SystemPhase::PostRender,
    8,
    run
)