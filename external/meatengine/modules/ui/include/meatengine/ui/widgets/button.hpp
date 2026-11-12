#pragma once
#include <entt/entt.hpp>
#include <string>
#include <meatengine/ui/widgets/base_node.hpp>
#include <meatengine/ui/widgets/label.hpp>
#include <meatengine/ui/cmp/Node.hpp>
#include <meatengine/ui/cmp/UiRect.hpp>
#include <meatengine/ui/cmp/FlexStyle.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>
#include <meatengine/ui/cmp/BackgroundColor.hpp>

namespace me::ui::widgets {
inline entt::entity make_button(entt::registry& reg) {
    auto e = make_base_node(reg);
    
    auto& node = reg.get<me::ui::cmp::Node>(e);
    node.width = me::ui::cmp::Val::Px(250.f);
    node.height = me::ui::cmp::Val::Px(50.f);
    node.padding = me::ui::cmp::UiRect::all(me::ui::cmp::Val::Px(4));

    auto& flex_style = reg.get<me::ui::cmp::FlexStyle>(e);
    flex_style.direction = me::ui::cmp::FlexDirection::Row;
    flex_style.justify = me::ui::cmp::JustifyContent::Center;
    flex_style.align_items = me::ui::cmp::AlignItems::Center;

    auto& fill_rect = reg.get<me::ui::cmp::FillRect>(e);
    fill_rect.styled = true;

    auto& background_color = reg.get<me::ui::cmp::BackgroundColor>(e);
    background_color.color = {120, 120, 120};

    return e;
}
inline entt::entity make_button(entt::registry& reg, const std::string& text) {
    auto e = make_button(reg);
    make_label(reg, e, text);
    return e;
}
}