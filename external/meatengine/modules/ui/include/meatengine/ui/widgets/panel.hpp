#pragma once
#include <entt/entt.hpp>
#include <string>
#include <meatengine/ui/widgets/base_node.hpp>
#include <meatengine/ui/cmp/Label.hpp>

namespace me::ui::widgets {
inline entt::entity make_panel(entt::registry& reg) {
    auto e = make_base_node(reg);
    reg.emplace<me::ui::cmp::Node>(e);
    reg.emplace<me::ui::cmp::FlexStyle>(e);
    reg.emplace<me::ui::cmp::FillRect>(e).styled = true;
    reg.emplace<me::ui::cmp::BackgroundColor>(e);
    reg.emplace<me::ui::cmp::BorderColor>(e);
    return e;
}

}