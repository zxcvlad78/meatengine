#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::FillRect>("FillRect",
        sol::constructors<me::ui::cmp::FillRect()>(),
        "shape", &me::ui::cmp::FillRect::shape,
        "background", &me::ui::cmp::FillRect::background,
        "styled", &me::ui::cmp::FillRect::styled,
        "dirty", &me::ui::cmp::FillRect::dirty
    );

    me::lua::register_component<me::ui::cmp::FillRect>("FillRect", lua);
}

ME_LUA_BINDING(
    "engine:cmp:ui:FillRect",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)