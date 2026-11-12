#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/spatial/cmp/GlobalTransform.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::spatial::cmp::GlobalTransform>("GlobalTransform", sol::no_constructor);

    me::lua::register_component<me::spatial::cmp::GlobalTransform>("GlobalTransform", lua);
}

ME_LUA_BINDING(
    "engine:cmp:GlobalTransform",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)