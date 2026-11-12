#pragma once

#include <entt/entt.hpp>
#include <string>

#include <meatengine/hierarchy/cmp/Children.hpp>
#include <meatengine/ui/cmp/Label.hpp>

namespace me::ui {

    inline void set_button_text(entt::registry& r, entt::entity button, const std::string& text) {
        auto* ch = r.try_get<me::hierarchy::cmp::Children>(button);
        if (!ch) return;

        for (auto c : ch->entities) {
            if (auto* lb = r.try_get<me::ui::cmp::Label>(c); lb && lb->text) {
                lb->set_string(text);
                return;
            }
        }
    }

} // namespace me::ui
