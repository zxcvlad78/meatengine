#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::Font>("Font", sol::no_constructor);
    
    sol::table rl = lua["ResourceLoader"];
    me::lua::register_resource<me::Font>(rl, "Font");
}

ME_LUA_BINDING(
    "engine:Font",
    static_cast<unsigned>(me::lua::BindPriority::Resource),
    &bind
)