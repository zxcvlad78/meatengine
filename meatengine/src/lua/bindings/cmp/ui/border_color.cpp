#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/BorderColor.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::BorderColor>("BorderColor",
        sol::constructors<me::cmp::ui::BorderColor()>(),
        "color", &me::cmp::ui::BorderColor::color
    );

    me::lua::register_component<me::cmp::ui::BorderColor>("BorderColor", lua);
}

ME_LUA_BINDING(
    "engine:ui:BorderColor",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)