#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/Transform.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::Transform>("Transform",
        sol::constructors<me::cmp::Transform()>(),
        "position", &me::cmp::Transform::position,
        "rotation", &me::cmp::Transform::rotation,
        "scale", &me::cmp::Transform::scale
    );
    
    me::lua::register_component<me::cmp::Transform>("Transform", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Transform",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)