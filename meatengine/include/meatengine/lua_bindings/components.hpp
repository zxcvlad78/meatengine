#pragma once

#include <sol/sol.hpp>
#include <entt/entt.hpp>

#include <functional>
#include <stdexcept>
#include <string>

#include "meatengine/ResourceLoader.hpp"

namespace me::lua_bindings {
    inline sol::usertype<entt::registry> get_reg_type(sol::state& lua) {
        sol::object existing = lua["Registry"];
        if (existing.valid() && existing.is<sol::table>()) {
            return sol::usertype<entt::registry>(existing.as<sol::table>());
        }

        auto ut = lua.new_usertype<entt::registry>("Registry",
            sol::constructors<entt::registry()>()
        );
        ut.set("create",  [](entt::registry& r) { return r.create(); });
        ut.set("destroy", [](entt::registry& r, entt::entity e) { r.destroy(e); });
        return ut;
    }

    template <typename Component>
    void register_component_methods(const std::string& name, sol::state& lua) {
        auto reg_type = get_reg_type(lua);
        reg_type.set("add_" + name,
            [](entt::registry& r, entt::entity e) -> Component& {
                return r.get_or_emplace<Component>(e);
            });

        reg_type.set("get_" + name,
            [](entt::registry& r, entt::entity e) -> Component& {
                return r.get<Component>(e);
            });

        reg_type.set("has_" + name,
            [](entt::registry& r, entt::entity e) -> bool {
                return r.all_of<Component>(e);
            });

        reg_type.set("remove_" + name,
            [](entt::registry& r, entt::entity e) {
                r.remove<Component>(e);
            });
    }

    template <typename Component>
    void register_view_method(const std::string& name, sol::state& lua) {
        auto reg_type = get_reg_type(lua);
        reg_type.set("for_each_" + name,
            [](entt::registry& r, sol::function callback) {
                auto view = r.view<Component>();
                for (auto [entity, c] : view) {
                    callback(entity, std::ref(c));
                }
            });
    }

    template <typename Component>
    void register_tag(const std::string& name, sol::state& lua) {
        auto reg_type = get_reg_type(lua);
        reg_type.set("add_" + name,
            [](entt::registry& r, entt::entity e) {
                if (!r.any_of<Component>(e))
                    r.emplace<Component>(e);
            });

        reg_type.set("has_" + name,
            [](entt::registry& r, entt::entity e) -> bool {
                return r.all_of<Component>(e);
            });

        reg_type.set("remove_" + name,
            [](entt::registry& r, entt::entity e) {
                r.remove<Component>(e);
            });
    }

    template <typename Component>
    void register_component(const std::string& name, sol::state& lua) {
        register_component_methods<Component>(name, lua);
        register_view_method<Component>(name, lua);
    }

    template <typename T>
    void bind_resource(sol::table& table, const std::string& name) {
        table.set_function("load_" + name,
            [name](const std::string& path) -> T& {
                auto res = me::ResourceLoader::load<T>(path);
                auto h = res.handle();
                if (!h) throw std::runtime_error("[ResourceLoader] load_" + name + " failed: " + path);
                return *h;
            });

        table.set_function("get_" + name,
            [](const std::string& path) -> T* {
                auto res = me::ResourceLoader::get<T>(path);
                auto h = res.handle();
                return h ? h.get() : nullptr;
            });
    }
} // namespace me::lua_bindings