#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/BorderWidth.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::BorderWidth>("BorderWidth",
        sol::constructors<me::ui::cmp::BorderWidth()>(),
        "width", &me::ui::cmp::BorderWidth::width
    );

    me::lua::register_component<me::ui::cmp::BorderWidth>("BorderWidth", lua);
}

ME_LUA_BINDING(
    "engine:ui:BorderWidth",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)