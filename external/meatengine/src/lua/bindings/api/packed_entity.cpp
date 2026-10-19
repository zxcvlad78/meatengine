#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/PackedEntity.hpp>

static void bind(sol::state& lua) {
    sol::table table = lua.create_table();
    lua["PackedEntity"] = table;

    table.set_function("register",
        [](const std::string& id, sol::protected_function fn) {
            me::PackedEntity::register_pe(id, fn);
        });
    
    table.set_function("spawn",
        [](const std::string& id, entt::registry& reg) -> entt::entity {
            return me::PackedEntity::spawn(id, reg);
        });
    
    table.set_function("exists",
        [](const std::string& id) -> bool {
            return me::PackedEntity::exists(id);
        });
}

ME_LUA_BINDING(
    "engine:api:PackedEntity",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)