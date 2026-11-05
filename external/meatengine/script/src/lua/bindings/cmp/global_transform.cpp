#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/GlobalTransform.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::GlobalTransform>("GlobalTransform", sol::no_constructor);

    me::lua::register_component<me::cmp::GlobalTransform>("GlobalTransform", lua);
}

ME_LUA_BINDING(
    "engine:cmp:GlobalTransform",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)