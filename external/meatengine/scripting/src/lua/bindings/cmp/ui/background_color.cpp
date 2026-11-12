#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/BackgroundColor.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::BackgroundColor>("BackgroundColor",
        sol::constructors<me::ui::cmp::BackgroundColor()>(),
        "color", &me::ui::cmp::BackgroundColor::color
    );

    me::lua::register_component<me::ui::cmp::BackgroundColor>("BackgroundColor", lua);
}

ME_LUA_BINDING(
    "engine:ui:BackgroundColor",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)