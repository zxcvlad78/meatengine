#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/Camera.hpp>

static void bind(sol::state& lua) {
    auto usertype = lua.new_usertype<me::cmp::Camera>("Camera",
        sol::constructors<me::cmp::Camera()>(),
        "zoom", &me::cmp::Camera::zoom,
        "smooth", &me::cmp::Camera::smooth
    );

    usertype.set_function("is_current", &me::cmp::Camera::is_current);
    usertype.set_function("set_current", &me::cmp::Camera::set_current);
    usertype.set_function("make_current", &me::cmp::Camera::make_current);
    usertype.set_function("get_current", &me::cmp::Camera::get_current);
    
    me::lua::register_component<me::cmp::Camera>("Camera", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Camera",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)