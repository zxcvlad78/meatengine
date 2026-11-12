#include <string>
#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/assets/ResourceLoader.hpp>
#include <meatengine/assets/lua/ResourceBinder.hpp>

#include <meatengine/assets/Resources.hpp>

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