#include <sol/sol.hpp>

#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/td/cmp/Transform3D.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::td::cmp::Transform3D>("Transform3D",
        sol::constructors<me::td::cmp::Transform3D()>(),
        "position", &me::td::cmp::Transform3D::position,
        "rotation", &me::td::cmp::Transform3D::rotation,
        "scale", &me::td::cmp::Transform3D::scale
    );

    me::lua::register_component<me::td::cmp::Transform3D>("Transform3D", lua);
}

ME_LUA_BINDING(
    "engine:3d:cmp:Transform3D",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)
