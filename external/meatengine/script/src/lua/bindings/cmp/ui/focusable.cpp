#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/Focusable.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::Focusable>("Focusable",
        sol::constructors<me::cmp::ui::Focusable()>()
    );

    me::lua::register_tag<me::cmp::ui::Focusable>("Focusable", lua);
}

ME_LUA_BINDING(
    "engine:ui:Focusable",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)