#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/Children.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::Children>("Children", sol::no_constructor);

    me::lua::register_component<me::cmp::Children>("Children", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Children",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)