#pragma once
#include <entt/entt.hpp>
#include <string>
#include <meatengine/ui/widgets/base_node.hpp>
#include <meatengine/ui/cmp/Label.hpp>

namespace me::ui::widgets {
inline entt::entity make_label(entt::registry& reg) {
    auto e = make_base_node(reg);
    auto& label = reg.emplace<me::ui::cmp::Label>(e);
    return e;
}
inline entt::entity make_label(entt::registry& reg, entt::entity parent) {
    auto e = make_label(reg);
    me::hierarchy::set_parent(reg, e, parent);
    return e;
}

inline entt::entity make_label(entt::registry& reg, const std::string& text) {
    auto e = make_label(reg);
    auto& label = reg.get<me::ui::cmp::Label>(e);
    label.set_string(text);
    return e;
}
inline entt::entity make_label(entt::registry& reg, entt::entity parent, const std::string& text) {
    auto e = make_label(reg, parent);
    auto& label = reg.get<me::ui::cmp::Label>(e);
    label.set_string(text);
    return e;
}
}