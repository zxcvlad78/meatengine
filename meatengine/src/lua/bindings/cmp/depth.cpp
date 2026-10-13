#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/Depth.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::Depth>("Depth",
        sol::constructors<me::cmp::Depth()>(),
        "value", &me::cmp::Depth::value
    );

    me::lua::register_component<me::cmp::Depth>("Depth", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Depth",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)