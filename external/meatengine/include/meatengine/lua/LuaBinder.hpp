#pragma once
#include <sol/sol.hpp>
#include <functional>
#include <string>
#include <vector>
#include <entt/entt.hpp>
#include <meatengine/ResourceLoader.hpp>
#include <meatengine/lua/ComponentDescriptor.hpp>

namespace me::lua {

enum class BindPriority : unsigned int {
    BaseType = 0,
    API = 1000,
    Resource = 2000,
    Component = 3000,
    
    Invalid = std::numeric_limits<int>::max()
};

inline const char* to_string(BindPriority p) {
    switch (p) {
        case BindPriority::BaseType: return "BaseType";
        case BindPriority::API: return "API";
        case BindPriority::Resource: return "Resource";
        case BindPriority::Component: return "Component";
        default: return "Invalid";
    }
}

inline BindPriority from_string(std::string_view s) {
    if (s == "BaseType") return BindPriority::BaseType;
    if (s == "API") return BindPriority::API;
    if (s == "Resource") return BindPriority::Resource;
    if (s == "Component") return BindPriority::Component;
    return BindPriority::Invalid;
}

struct BindingEntry { std::string name; unsigned int priority = 0; std::function<void(sol::state&)> func; };
std::vector<BindingEntry>& binding_list();

void bind(sol::state& lua);

struct BindingAutoReg {
    BindingAutoReg(std::string name, unsigned int priority, std::function<void(sol::state&)> fn);
};

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
void register_component_view(const std::string& name, sol::state& lua) {
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
    register_component_view<Component>(name, lua);

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
void register_resource(sol::table& table, const std::string& name) {
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

} // namespace me::lua

#define ME_LUA_BINDING_IMPL(NAME, PRIORITY, FN, CNT) \
    namespace { \
        const ::me::lua::BindingAutoReg _me_lua_bind_##CNT{ \
            NAME, PRIORITY, (FN) \
        }; \
    }

#define ME_LUA_BINDING_IMPL_VA(NAME, PRIORITY, CNT, ...) \
    namespace { \
        const ::me::lua::BindingAutoReg _me_lua_bind_##CNT{ \
            NAME, PRIORITY, (__VA_ARGS__) \
        }; \
    }

#define ME_LUA_BINDING(NAME, PRIORITY, ...) \
    ME_LUA_BINDING_IMPL_VA(NAME, PRIORITY, __COUNTER__, __VA_ARGS__)

#define ME_LUA_BINDING_FN(NAME, PRIORITY, FN) \
    ME_LUA_BINDING_IMPL(NAME, PRIORITY, (FN), __COUNTER__)