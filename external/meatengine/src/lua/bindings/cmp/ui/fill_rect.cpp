#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::FillRect>("FillRect",
        sol::constructors<me::cmp::ui::FillRect()>(),
        "shape", &me::cmp::ui::FillRect::shape,
        "background", &me::cmp::ui::FillRect::background,
        "styled", &me::cmp::ui::FillRect::styled,
        "dirty", &me::cmp::ui::FillRect::dirty
    );

    me::lua::register_component<me::cmp::ui::FillRect>("FillRect", lua);
}

ME_LUA_BINDING(
    "engine:cmp:ui:FillRect",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)