#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/ScriptedStateRegistry.hpp>

static void bind(sol::state& lua) {
    sol::table table = lua.create_table();
    lua["ScriptedStateRegistry"] = table;

    table.set_function("register",
        [](const std::string& id, sol::table callbacks) {
            me::ScriptedStateRegistry::register_ss(id, callbacks);
        });

    table.set_function("change_to",
        [](const std::string& id) {
            me::ScriptedStateRegistry::change_to(id);
        });

    table.set_function("exists",
        [](const std::string& id) -> bool {
            return me::ScriptedStateRegistry::exists(id);
        });
}

ME_LUA_BINDING(
    "engine:api:ScriptedStateRegistry",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)