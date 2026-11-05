#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>
#include <meatengine/ResourceLoader.hpp>

static void bind(sol::state& lua) {
    sol::table rl = lua.create_table();
    lua["ResourceLoader"] = rl;
}

ME_LUA_BINDING(
    "engine:api:ResourceLoader",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)