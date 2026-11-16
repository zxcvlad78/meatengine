#pragma once
#include <entt/entt.hpp>
#include <string>
#include <meatengine/hierarchy/hierarchy.hpp>
#include <meatengine/ui/widgets/base_node.hpp>
#include <meatengine/ui/cmp/Node.hpp>
#include <meatengine/ui/cmp/FlexStyle.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>
#include <meatengine/ui/cmp/BackgroundColor.hpp>
#include <meatengine/ui/cmp/Focusable.hpp>
#include <meatengine/ui/cmp/Interaction.hpp>
#include <meatengine/ui/cmp/TextEditState.hpp>

namespace me::ui::widgets {
inline entt::entity make_text_edit(entt::registry& reg) {
    auto e = make_base_node(reg);

    auto& node = reg.get<me::ui::cmp::Node>(e);
    node.width = me::ui::cmp::Val::Px(320.f);
    node.height = me::ui::cmp::Val::Px(160.f);
    node.padding = me::ui::cmp::UiRect::all(me::ui::cmp::Val::Px(6.f));

    auto& flex_style = reg.get<me::ui::cmp::FlexStyle>(e);
    flex_style.direction = me::ui::cmp::FlexDirection::Column;
    flex_style.align_items = me::ui::cmp::AlignItems::FlexStart;

    reg.emplace<me::ui::cmp::FillRect>(e);
    reg.emplace<me::ui::cmp::BackgroundColor>(e).color = sf::Color::White;
    reg.emplace<me::ui::cmp::Focusable>(e);
    reg.emplace<me::ui::cmp::Interaction>(e);

    auto& st = reg.emplace<me::ui::cmp::TextEditState>(e);
    st.multiline = true;
    return e;
}

inline entt::entity make_text_edit(entt::registry& reg, const std::string& text) {
    auto e = make_text_edit(reg);
    reg.get<me::ui::cmp::TextEditState>(e).set_value_utf8(text);
    return e;
}

inline entt::entity make_text_edit(entt::registry& reg, entt::entity parent) {
    auto e = make_text_edit(reg);
    me::hierarchy::set_parent(reg, e, parent);
    return e;
}
inline entt::entity make_text_edit(entt::registry& reg, entt::entity parent, const std::string& text) {
    auto e = make_text_edit(reg, parent);
    reg.get<me::ui::cmp::TextEditState>(e).set_value_utf8(text);
    return e;
}
}
