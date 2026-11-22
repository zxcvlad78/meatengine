#include <sol/sol.hpp>

#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/td/MaterialOps.hpp>
#include <meatengine/td/cmp/Material3D.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::td::cmp::Material3D>("Material3D",
        sol::constructors<me::td::cmp::Material3D()>(),
        "albedo", &me::td::cmp::Material3D::albedo,
        "emissive", &me::td::cmp::Material3D::emissive,
        "shininess", &me::td::cmp::Material3D::shininess,
        "specular", &me::td::cmp::Material3D::specular,
        "opacity", &me::td::cmp::Material3D::opacity,
        "unlit", &me::td::cmp::Material3D::unlit,
        "two_sided", &me::td::cmp::Material3D::two_sided
    );

    me::lua::register_component<me::td::cmp::Material3D>("Material3D", lua);
}

ME_LUA_BINDING(
    "engine:3d:cmp:Material3D",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)
