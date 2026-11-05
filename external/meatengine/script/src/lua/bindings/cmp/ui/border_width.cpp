#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/BorderWidth.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::BorderWidth>("BorderWidth",
        sol::constructors<me::cmp::ui::BorderWidth()>(),
        "width", &me::cmp::ui::BorderWidth::width
    );

    me::lua::register_component<me::cmp::ui::BorderWidth>("BorderWidth", lua);
}

ME_LUA_BINDING(
    "engine:ui:BorderWidth",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)