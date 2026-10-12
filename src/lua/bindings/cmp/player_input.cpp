#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <godlike/cmp/PlayerInput.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<godlike::cmp::PlayerInput>("PlayerInput", sol::no_constructor);
    me::lua::register_tag<godlike::cmp::PlayerInput>("PlayerInput", lua);
}

ME_LUA_BINDING(
    "godlike:cmp:PlayerInput",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)
