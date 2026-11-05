#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/ModLoader.hpp>

static void bind(sol::state& lua) {
    sol::table table = lua.create_table();
    lua["ModLoader"] = table;

    table.set_function("list", []() {
        return me::ModLoader::list();
    });

    // add reload mod 
}

ME_LUA_BINDING(
    "engine:api:ModLoader",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)