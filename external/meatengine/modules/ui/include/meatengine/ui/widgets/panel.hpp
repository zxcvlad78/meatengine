#pragma once
#include <entt/entt.hpp>
#include <string>
#include <meatengine/hierarchy/hierarchy.hpp>
#include <meatengine/ui/widgets/base_node.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>
#include <meatengine/ui/cmp/BackgroundColor.hpp>
#include <meatengine/ui/cmp/BorderColor.hpp>

namespace me::ui::widgets {
inline entt::entity make_panel(entt::registry& reg) {
    auto e = make_base_node(reg);
    reg.emplace<me::ui::cmp::FillRect>(e);//.styled = true;
    reg.emplace<me::ui::cmp::BackgroundColor>(e);
    reg.emplace<me::ui::cmp::BorderColor>(e);
    return e;
}

inline entt::entity make_panel(entt::registry& reg, entt::entity parent) {
    auto e = make_panel(reg);
    me::hierarchy::set_parent(reg, e, parent);
    return e;
}

}