#include <string>
#include <meatengine/scripting/LuaBinder.hpp>

#include <meatengine/graphics/cmp/Camera.hpp>

static void bind(sol::state& lua) {
    auto usertype = lua.new_usertype<me::graphics::cmp::Camera>("Camera",
        sol::constructors<me::graphics::cmp::Camera()>(),
        "zoom", &me::graphics::cmp::Camera::zoom,
        "smooth", &me::graphics::cmp::Camera::smooth
    );

    usertype.set_function("is_current", &me::graphics::cmp::Camera::is_current);
    usertype.set_function("set_current", &me::graphics::cmp::Camera::set_current);
    usertype.set_function("make_current", &me::graphics::cmp::Camera::make_current);
    usertype.set_function("get_current", &me::graphics::cmp::Camera::get_current);
    
    me::lua::register_component<me::graphics::cmp::Camera>("Camera", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Camera",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)