#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/hierarchy.hpp>

static void bind(sol::state& lua) {
    sol::table h = lua.create_table();
    lua["Hierarchy"] = h;

    h.set_function("set_parent", [](entt::registry& r, entt::entity child, entt::entity parent) {
        me::hierarchy::set_parent(r, child, parent);
    });
    h.set_function("clear_parent", [](entt::registry& r, entt::entity child) {
        me::hierarchy::clear_parent(r, child);
    });
}

ME_LUA_BINDING(
    "engine:api:Hierarchy",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)