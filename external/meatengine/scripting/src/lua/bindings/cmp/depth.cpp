#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/abstract/cmp/Depth.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::abstract::cmp::Depth>("Depth",
        sol::constructors<me::abstract::cmp::Depth()>(),
        "value", &me::abstract::cmp::Depth::value
    );

    me::lua::register_component<me::abstract::cmp::Depth>("Depth", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Depth",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)