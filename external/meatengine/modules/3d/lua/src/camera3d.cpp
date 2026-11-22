#include <sol/sol.hpp>

#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/td/cmp/Camera3D.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::td::cmp::Camera3D>("Camera3D",
        sol::constructors<me::td::cmp::Camera3D()>(),
        "fov", &me::td::cmp::Camera3D::fov_deg,
        "near", &me::td::cmp::Camera3D::near_z,
        "far", &me::td::cmp::Camera3D::far_z,
        "perspective", &me::td::cmp::Camera3D::perspective,
        "ortho_size", &me::td::cmp::Camera3D::ortho_size,
        "current", &me::td::cmp::Camera3D::current
    );

    me::lua::register_component<me::td::cmp::Camera3D>("Camera3D", lua);
}

ME_LUA_BINDING(
    "engine:3d:cmp:Camera3D",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)
