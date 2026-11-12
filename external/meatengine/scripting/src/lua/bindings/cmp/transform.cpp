#include <string>
#include <meatengine/scripting/LuaBinder.hpp>

#include <meatengine/spatial/cmp/Transform.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::spatial::cmp::Transform>("Transform",
        sol::constructors<me::spatial::cmp::Transform()>(),
        "position", &me::spatial::cmp::Transform::position,
        "rotation", &me::spatial::cmp::Transform::rotation,
        "scale", &me::spatial::cmp::Transform::scale
    );
    
    me::lua::register_component<me::spatial::cmp::Transform>("Transform", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Transform",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)