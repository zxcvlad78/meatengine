#pragma once
#include <entt/entt.hpp>
#include <meatengine/hierarchy/cmp/Children.hpp>
#include <meatengine/ui/cmp/Node.hpp>
#include <meatengine/ui/cmp/FlexStyle.hpp>

namespace me::ui::widgets {
inline entt::entity make_base_node(entt::registry& reg) {
    auto e = reg.create();
    reg.emplace<me::spatial::cmp::Transform>(e);
    reg.emplace<me::hierarchy::cmp::Children>(e);
    reg.emplace<me::ui::cmp::Node>(e);
    reg.emplace<me::ui::cmp::FlexStyle>(e);
    return e;
}
}