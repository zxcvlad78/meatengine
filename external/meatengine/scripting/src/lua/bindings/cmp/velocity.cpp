#include <string>
#include <meatengine/scripting/LuaBinder.hpp>

#include <meatengine/physics/cmp/Velocity.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::physics::cmp::Velocity>("Velocity",
        sol::constructors<me::physics::cmp::Velocity()>(),
        "linear", &me::physics::cmp::Velocity::linear,
        "angular", &me::physics::cmp::Velocity::angular
    );
    
    me::lua::register_component<me::physics::cmp::Velocity>("Velocity", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Velocity",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)