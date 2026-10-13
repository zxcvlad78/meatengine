#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/GlobalBounds.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::GlobalBounds>("GlobalBounds", sol::no_constructor);

    me::lua::register_component<me::cmp::GlobalBounds>("GlobalBounds", lua);
}

ME_LUA_BINDING(
    "engine:cmp:GlobalBounds",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)