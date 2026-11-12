#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/app/ctx/InputState.hpp>

#include <meatengine/hierarchy/cmp/Children.hpp>

#include <meatengine/ui/cmp/UIRoot.hpp>
#include <meatengine/ui/cmp/ComputedNode.hpp>
#include <meatengine/ui/cmp/Interaction.hpp>
#include <meatengine/ui/cmp/Focusable.hpp>
#include <meatengine/ui/cmp/States.hpp>

#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;

struct FocusState {
    entt::entity focused = entt::null;
    std::vector<entt::entity> order;
    size_t index = 0;
    bool tab_was_down = false;
};

void collect(entt::registry& r, entt::entity e, std::vector<entt::entity>& out) {
    if (r.any_of<me::ui::cmp::Focusable>(e))
        out.push_back(e);
    if (auto* ch = r.try_get<me::hierarchy::cmp::Children>(e))
        for (auto c : ch->entities)
            if (r.valid(c)) collect(r, c, out);
}

void set_focused(entt::registry& r, entt::entity e) {
    auto& fs = r.ctx().get<FocusState>();
    if (fs.focused == e) return;
    if (fs.focused != entt::null && r.valid(fs.focused))
        r.remove<me::ui::cmp::Focused>(fs.focused);
    
    fs.focused = e;
    if (e != entt::null) r.emplace_or_replace<me::ui::cmp::Focused>(e);
}

void run() {
    auto& r = me::MainLoop::get_registry();
    if (!r.ctx().contains<FocusState>())
        r.ctx().emplace<FocusState>();
    
    auto& fs = r.ctx().get<FocusState>();

    fs.order.clear();
    for (auto [e] : r.view<me::ui::cmp::UIRoot>().each())
        collect(r, e, fs.order);

    if (fs.order.empty()) {
        fs.tab_was_down = false;
        return;
    }

    auto it = std::find(fs.order.begin(), fs.order.end(), fs.focused);
    if (it == fs.order.end()) {
        fs.focused = entt::null;
    } else {
        fs.index = static_cast<size_t>(it - fs.order.begin());
    }


    bool tab_down   = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab);
    bool shift_down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);

    if (tab_down && !fs.tab_was_down) {
        int dir = shift_down ? -1 : 1;
        int n = static_cast<int>(fs.order.size());
        int i = static_cast<int>(fs.index);
        i = (i + dir + n) % n;
        fs.index = static_cast<size_t>(i);
        set_focused(r, fs.order[fs.index]);
    }
    fs.tab_was_down = tab_down;

    auto& in = r.ctx().get<me::app::ctx::InputState>();
    if (in.mouse_just_pressed) {
        entt::entity best = entt::null;
        uint32_t best_stack = 0;
        for (auto e : fs.order) {
            if (!r.any_of<me::ui::cmp::Hovered>(e)) continue;
            uint32_t s = 0;
            if (auto* cn = r.try_get<me::ui::cmp::ComputedNode>(e))
                s = cn->stack_index;
            if (best == entt::null || s >= best_stack) {
                best = e;
                best_stack = s;
            }
        }
        if (best != entt::null) {
            auto it2 = std::find(fs.order.begin(), fs.order.end(), best);
            if (it2 != fs.order.end()) {
                fs.index = static_cast<size_t>(it2 - fs.order.begin());
                set_focused(r, best);
            }
        }
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:focus",
    me::SystemPhase::Input,
    4,
    run
)