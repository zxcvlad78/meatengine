#pragma once
#include <entt/entity/registry.hpp>
#include <meatengine/hierarchy/cmp/Parent.hpp>
#include <meatengine/hierarchy/cmp/Children.hpp>
#include <algorithm>

namespace me::hierarchy {
inline void set_parent(entt::registry& r, entt::entity child, entt::entity parent) {
    if (child == parent) return;

    if (auto* p = r.try_get<me::hierarchy::cmp::Parent>(child)) {
        if (p->entity != entt::null && r.valid(p->entity)) {
            if (auto* ch = r.try_get<me::hierarchy::cmp::Children>(p->entity)) {
                auto& v = ch->entities;
                v.erase(std::remove(v.begin(), v.end(), child), v.end());
            }
        }
    }

    if (parent == entt::null) {
        r.remove<me::hierarchy::cmp::Parent>(child);
        return;
    }

    r.emplace_or_replace<me::hierarchy::cmp::Parent>(child, parent);
    auto& ch = r.get_or_emplace<me::hierarchy::cmp::Children>(parent);
    if (std::find(ch.entities.begin(), ch.entities.end(), child) == ch.entities.end())
        ch.entities.push_back(child);
}

inline void clear_parent(entt::registry& r, entt::entity child) {
    set_parent(r, child, entt::null);
}
}