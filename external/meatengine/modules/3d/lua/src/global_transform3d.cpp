#include <sol/sol.hpp>

#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/td/cmp/GlobalTransform3D.hpp>

// Read-mostly world matrix; computed by the engine:3d:transform system.

static void bind(sol::state& lua) {
    lua.new_usertype<me::td::cmp::GlobalTransform3D>("GlobalTransform3D",
        sol::constructors<me::td::cmp::GlobalTransform3D()>(),
        "matrix", &me::td::cmp::GlobalTransform3D::matrix
    );

    me::lua::register_component<me::td::cmp::GlobalTransform3D>("GlobalTransform3D", lua);
}

ME_LUA_BINDING(
    "engine:3d:cmp:GlobalTransform3D",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)
