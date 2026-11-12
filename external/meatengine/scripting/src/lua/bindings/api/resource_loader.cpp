#include <string>
#include <meatengine/scripting/LuaBinder.hpp>

#include <meatengine/assets/Resources.hpp>
#include <meatengine/assets/ResourceLoader.hpp>

static void bind(sol::state& lua) {
    sol::table rl = lua.create_table();
    lua["ResourceLoader"] = rl;
}

ME_LUA_BINDING(
    "engine:api:ResourceLoader",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)