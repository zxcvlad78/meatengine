#pragma once

#include <sol/sol.hpp>
#include <entt/entt.hpp>

#include <functional>
#include <stdexcept>
#include <string>

#include "meatengine/ResourceLoader.hpp"
#include "meatengine/lua_bindings/ComponentDescriptor.hpp"

namespace me::lua_bindings {
    inline sol::usertype<entt::registry> get_reg_type(sol::state& lua) {
        sol::object existing = lua["EnttRegistry"];
        if (existing.valid() && existing.is<sol::table>()) {
            return sol::usertype<entt::registry>(existing.as<sol::table>());
        }

        auto ut = lua.new_usertype<entt::registry>("EnttRegistry",
            sol::no_constructor
        );
        ut.set("create", [](entt::registry& r) { return r.create(); });
        ut.set("destroy", [](entt::registry& r, entt::entity e) { r.destroy(e); });
        ut.set("valid", [](entt::registry& r, entt::entity e) { return r.valid(e); });

        ut.set_function("foreach",
            [](entt::registry& r, sol::table names, sol::protected_function cb) {

                std::vector<const ComponentDescriptor*> descs;
                descs.reserve(names.size());

                for (auto&& [_, v] : names) {
                    if (!v.is<std::string>())
                        throw std::runtime_error("foreach: component name must be a string");

                    const std::string name = v.as<std::string>();
                    auto it = descriptors().find(name);
                    if (it == descriptors().end())
                        throw std::runtime_error("foreach: unknown component or tag '" + name + "'");

                    descs.push_back(&it->second);
                }

                if (descs.empty()) return;

                if (descs.size() == 1) {
                    descs[0]->foreach_single(r, std::move(cb));
                    return;
                }

                std::vector<std::string> sorted_names;
                sorted_names.reserve(descs.size());
                for (auto* d : descs) sorted_names.push_back(d->name);
                std::sort(sorted_names.begin(), sorted_names.end());

                std::string key;
                key.reserve(sorted_names.size() * 16);
                for (auto& n : sorted_names) { key += n; key += ','; }

                auto& cache = view_cache();
                auto cached = std::find_if(cache.begin(), cache.end(),
                    [&](const ViewCache& c) { return c.key == key; });

                if (cached == cache.end()) {
                    entt::runtime_view view{};

                    for (auto* d : descs) {
                        auto* storage = r.storage(d->type_id);
                        if (storage) {
                            view.iterate(*storage);
                        }
                    }

                    cache.emplace_back(key, std::move(view));
                    cached = cache.end() - 1;
                }

                lua_State* L = cb.lua_state();
                const int nargs = 1 + static_cast<int>(descs.size());

                for (auto e : cached->view) {
                    cb.push(); 
                    sol::stack::push(L, e); 

                    for (auto* d : descs)
                        d->push(L, r, e);

                    if (lua_pcall(L, nargs, 0, 0) != LUA_OK) {
                        std::string err = lua_tostring(L, -1);
                        lua_pop(L, 1);
                        throw std::runtime_error("[foreach] " + err);
                    }
                }
            });

        lua["EnttRegistry"] = ut;
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

        reg_type.set("foreach_" + name,
            [name](entt::registry& r, sol::protected_function cb) {
                auto view = r.view<Component>();

                for (auto entity : view) {
                    auto& component = view.template get<Component>(entity);

                    auto result = cb(entity, &component);

                    if (!result.valid()) {
                        sol::error err = result;
                        throw std::runtime_error("[foreach_" + name + "] " + err.what());
                    }
                }
            });
    }

    template <typename Tag>
    void register_tag(const std::string& name, sol::state& lua) {
        auto reg_type = get_reg_type(lua);

        reg_type.set("add_" + name,
            [](entt::registry& r, entt::entity e) {
                if (!r.any_of<Tag>(e))
                    r.emplace<Tag>(e);
            });

        reg_type.set("has_" + name,
            [](entt::registry& r, entt::entity e) -> bool {
                return r.all_of<Tag>(e);
            });

        reg_type.set("remove_" + name,
            [](entt::registry& r, entt::entity e) {
                r.remove<Tag>(e);
            });

        ComponentDescriptor d({
            name,
            entt::type_hash<Tag>::value(),
            true
        });

        d.push = [](lua_State* L, entt::registry&, entt::entity) { lua_pushnil(L); };
        d.add = [](entt::registry& r, entt::entity e) { r.get_or_emplace<Tag>(e); };
        d.has = [](entt::registry& r, entt::entity e) { return r.all_of<Tag>(e); };
        d.remove = [](entt::registry& r, entt::entity e) { r.remove<Tag>(e); };

        d.foreach_single = [](entt::registry& r, sol::protected_function cb) {
            auto view = r.view<Tag>();
            lua_State* L = cb.lua_state();

            for (auto entity : view) {
                cb.push();
                sol::stack::push(L, entity);
                lua_pushnil(L);

                if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
                    std::string err = lua_tostring(L, -1);
                    lua_pop(L, 1);
                    throw std::runtime_error("[foreach_single:tag] " + err);
                }
            }
        };

        descriptors()[name] = std::move(d);
    }

    template <typename Component>
    void register_component(const std::string& name, sol::state& lua) {
        register_component_methods<Component>(name, lua);
        register_view_method<Component>(name, lua);

        ComponentDescriptor d({
            name,
            entt::type_hash<Component>::value(),
            false
        });

        d.push = [](lua_State* L, entt::registry& r, entt::entity e) {
            if (!r.all_of<Component>(e)) { lua_pushnil(L); return; }
            sol::stack::push(L, &r.get<Component>(e));
        };

        d.has = [](entt::registry& r, entt::entity e) { return r.all_of<Component>(e); };
        d.add = [](entt::registry& r, entt::entity e) { r.get_or_emplace<Component>(e); };
        d.remove = [](entt::registry& r, entt::entity e) { r.remove<Component>(e); };

        d.foreach_single = [](entt::registry& r, sol::protected_function cb) {
            auto view = r.view<Component>();
            lua_State* L = cb.lua_state();

            for (auto entity : view) {
                cb.push();
                sol::stack::push(L, entity);
                sol::stack::push(L, &view.template get<Component>(entity));

                if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
                    std::string err = lua_tostring(L, -1);
                    lua_pop(L, 1);
                    throw std::runtime_error("[foreach_single] " + err);
                }
            }
        };

        descriptors()[name] = std::move(d);
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
}