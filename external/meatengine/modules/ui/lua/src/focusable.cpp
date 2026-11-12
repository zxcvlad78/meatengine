#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/Focusable.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::Focusable>("Focusable",
        sol::constructors<me::ui::cmp::Focusable()>()
    );

    me::lua::register_tag<me::ui::cmp::Focusable>("Focusable", lua);
}

ME_LUA_BINDING(
    "engine:ui:Focusable",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)