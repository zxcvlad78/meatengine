#pragma once

#include <entt/entt.hpp>
#include <string>

#include <meatengine/hierarchy/cmp/Children.hpp>
#include <meatengine/ui/cmp/Node.hpp>
#include <meatengine/ui/cmp/Val.hpp>
#include <meatengine/ui/cmp/Label.hpp>
#include <meatengine/ui/cmp/FlexStyle.hpp>

namespace me::ui {
// node
void set_node_width(entt::registry& reg, entt::entity e, const cmp::Val& width);
void set_node_height(entt::registry& reg, entt::entity e, const cmp::Val& height);
void set_node_size(entt::registry& reg, entt::entity e, const cmp::Val& width, const cmp::Val& height);

// flex style
void set_flex_style_direction(entt::registry& reg, entt::entity e, me::ui::cmp::FlexDirection direction);


// label
void set_label_text(entt::registry& r, entt::entity e, const std::string& text);

//
void set_text(entt::registry& r, entt::entity e, const std::string& text);

}
