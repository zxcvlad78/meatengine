#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/Velocity.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::Velocity>("Velocity",
        sol::constructors<me::cmp::Velocity()>(),
        "linear", &me::cmp::Velocity::linear,
        "angular", &me::cmp::Velocity::angular
    );
    
    me::lua::register_component<me::cmp::Velocity>("Velocity", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Velocity",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)