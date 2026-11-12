#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/BorderColor.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::BorderColor>("BorderColor",
        sol::constructors<me::ui::cmp::BorderColor()>(),
        "color", &me::ui::cmp::BorderColor::color
    );

    me::lua::register_component<me::ui::cmp::BorderColor>("BorderColor", lua);
}

ME_LUA_BINDING(
    "engine:ui:BorderColor",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)