#pragma once
#include <entt/entt.hpp>
#include <meatengine/ui/cmp/Node.hpp>
#include <meatengine/ui/cmp/FlexStyle.hpp>
#include <meatengine/ui/widgets/base_node.hpp>

namespace me::ui::widgets {
inline entt::entity make_ui_root(entt::registry& reg) {
    auto e = make_base_node(reg);
    auto& node = reg.get<me::ui::cmp::Node>(e);
    node.width = me::ui::cmp::Val::Percent(100.f);
    node.height = me::ui::cmp::Val::Percent(100.f);

    auto& flex_style = reg.get<me::ui::cmp::FlexStyle>(e);
    flex_style.direction = me::ui::cmp::FlexDirection::Column;
    flex_style.justify = me::ui::cmp::JustifyContent::Center;
    flex_style.align_items = me::ui::cmp::AlignItems::Center;
    
    return e;
}
}