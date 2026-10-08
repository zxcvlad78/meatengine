#pragma once

#include <entt/entt.hpp>
#include <sol/sol.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace me::lua_bindings {

struct ComponentDescriptor {
    std::string name;
    entt::id_type type_id = entt::null;
    bool is_tag  = false;

    std::function<void(lua_State*, entt::registry&, entt::entity)> push;
    std::function<bool(entt::registry&, entt::entity)> has;
    std::function<void(entt::registry&, entt::entity)> add;
    std::function<void(entt::registry&, entt::entity)> remove;
    std::function<void(entt::registry&, sol::protected_function)> foreach_single;
};

std::unordered_map<std::string, ComponentDescriptor>& descriptors();

struct ViewCache {
    std::string key;
    entt::runtime_view view;

    ViewCache(std::string k, entt::runtime_view v)
        :
        key(std::move(k)),
        view(std::move(v))
    {}
};

std::vector<ViewCache>& view_cache();
void reset_view_cache();
}