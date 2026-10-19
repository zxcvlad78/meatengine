#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/BorderRadius.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::BorderRadius>("BorderRadius",
        sol::constructors<me::cmp::ui::BorderRadius()>(),
        "top_left", &me::cmp::ui::BorderRadius::top_left,
        "top_right", &me::cmp::ui::BorderRadius::top_right,
        "bottom_right", &me::cmp::ui::BorderRadius::bottom_right,
        "bottom_left", &me::cmp::ui::BorderRadius::bottom_left
    );

    me::lua::register_component<me::cmp::ui::BorderRadius>("BorderRadius", lua);
}

ME_LUA_BINDING(
    "engine:ui:BorderRaduis",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)