#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/BackgroundColor.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::BackgroundColor>("BackgroundColor",
        sol::constructors<me::cmp::ui::BackgroundColor()>(),
        "color", &me::cmp::ui::BackgroundColor::color
    );

    me::lua::register_component<me::cmp::ui::BackgroundColor>("BackgroundColor", lua);
}

ME_LUA_BINDING(
    "engine:ui:BackgroundColor",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)