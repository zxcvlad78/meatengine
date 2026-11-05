#pragma once
#include <entt/entity/registry.hpp>
#include <meatengine/cmp/Parent.hpp>
#include <meatengine/cmp/Children.hpp>
#include <meatengine/cmp/ui/Label.hpp>
#include <algorithm>

namespace me::hierarchy {

    inline void set_parent(entt::registry& r, entt::entity child, entt::entity parent) {
        if (child == parent) return;

        if (auto* p = r.try_get<cmp::Parent>(child)) {
            if (p->entity != entt::null && r.valid(p->entity)) {
                if (auto* ch = r.try_get<cmp::Children>(p->entity)) {
                    auto& v = ch->entities;
                    v.erase(std::remove(v.begin(), v.end(), child), v.end());
                }
            }
        }

        if (parent == entt::null) {
            r.remove<cmp::Parent>(child);
            return;
        }

        r.emplace_or_replace<cmp::Parent>(child, parent);
        auto& ch = r.get_or_emplace<cmp::Children>(parent);
        if (std::find(ch.entities.begin(), ch.entities.end(), child) == ch.entities.end())
            ch.entities.push_back(child);
    }

    inline void clear_parent(entt::registry& r, entt::entity child) {
        set_parent(r, child, entt::null);
    }

    inline void set_button_text(entt::registry& r, entt::entity btn, const std::string& text) {
        auto* ch = r.try_get<me::cmp::Children>(btn);
        if (!ch) return;
        for (auto c : ch->entities)
            if (auto* lb = r.try_get<me::cmp::ui::Label>(c); lb && lb->text) {
                lb->set_string(text);
                return;
            }
    }

}