#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/GameStateRegistry.hpp>

static void bind(sol::state& lua) {
    sol::table table = lua.create_table();
    lua["GameStateRegistry"] = table;

    table.set_function("change_to",
        [](const std::string& id) {
            me::GameStateRegistry::change_to(id);
        });

    table.set_function("exists",
        [](const std::string& id) -> bool {
            return me::GameStateRegistry::exists(id);
        });
}

ME_LUA_BINDING(
    "engine:api:GameStateRegistry",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)