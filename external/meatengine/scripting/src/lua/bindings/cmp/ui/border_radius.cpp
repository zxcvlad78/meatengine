#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/BorderRadius.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::BorderRadius>("BorderRadius",
        sol::constructors<me::ui::cmp::BorderRadius()>(),
        "top_left", &me::ui::cmp::BorderRadius::top_left,
        "top_right", &me::ui::cmp::BorderRadius::top_right,
        "bottom_right", &me::ui::cmp::BorderRadius::bottom_right,
        "bottom_left", &me::ui::cmp::BorderRadius::bottom_left
    );

    me::lua::register_component<me::ui::cmp::BorderRadius>("BorderRadius", lua);
}

ME_LUA_BINDING(
    "engine:ui:BorderRaduis",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)