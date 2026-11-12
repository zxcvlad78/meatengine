#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/ui/cmp/ComputedNode.hpp>
#include <meatengine/spatial/cmp/GlobalTransform.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>
#include <meatengine/ui/cmp/Label.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;

void run() {
    auto& r = me::MainLoop::get_registry();
    auto& win = me::MainLoop::get_window();

    std::vector<entt::entity> order;
    for (auto [e, _] : r.view<me::ui::cmp::ComputedNode>().each())
        order.push_back(e);

    std::sort(order.begin(), order.end(), [&](entt::entity a, entt::entity b) {
        return r.get<me::ui::cmp::ComputedNode>(a).stack_index
            < r.get<me::ui::cmp::ComputedNode>(b).stack_index;
    });

    for (auto e : order) {
        sf::Transform xf = r.try_get<me::spatial::cmp::GlobalTransform>(e)
                        ? r.get<me::spatial::cmp::GlobalTransform>(e).transform
                        : sf::Transform{};

        if (auto* fr = r.try_get<me::ui::cmp::FillRect>(e))
            win.draw(fr->shape, xf);

        if (auto* lb = r.try_get<me::ui::cmp::Label>(e); lb && lb->text)
            win.draw(*lb->text, xf);
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:render",
    me::SystemPhase::PostRender,
    8,
    run
)