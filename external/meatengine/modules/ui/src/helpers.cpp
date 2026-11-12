#include <meatengine/ui/helpers.hpp>


namespace me::ui {
// node
void set_node_width(entt::registry& reg, entt::entity e, const cmp::Val& width) {
    if (auto* node = reg.try_get<me::ui::cmp::Node>(e)) {
        node->width = width;
    }
}
void set_node_height(entt::registry& reg, entt::entity e, const cmp::Val& height) {
    if (auto* node = reg.try_get<me::ui::cmp::Node>(e)) {
        node->height = height;
    }
}
void set_node_size(entt::registry& reg, entt::entity e, const cmp::Val& width, const cmp::Val& height) {
    if (auto* node = reg.try_get<me::ui::cmp::Node>(e)) {
        node->width = width;
        node->height = height;
    }
}

// flex style
void set_flex_style_direction(entt::registry& reg, entt::entity e, me::ui::cmp::FlexDirection direction) {
    if (auto* flex_style = reg.try_get<me::ui::cmp::FlexStyle>(e)) {
        flex_style->direction = direction;
    }
}

// label
void set_label_text(entt::registry& r, entt::entity e, const std::string& text) {
    if (auto* lb = r.try_get<me::ui::cmp::Label>(e)) {
        lb->set_string(text);
    }
}


void set_text(entt::registry& r, entt::entity e, const std::string& text) {
    auto* c = r.try_get<me::hierarchy::cmp::Children>(e);
    if (!c) return;

    for (auto c : c->entities) {
        if (auto* lb = r.try_get<me::ui::cmp::Label>(c); lb && lb->text) {
            lb->set_string(text);
            return;
        }
    }
}

}
