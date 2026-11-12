#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/UIRoot.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::UIRoot>("UIRoot", sol::no_constructor);
    me::lua::register_tag<me::ui::cmp::UIRoot>("UIRoot", lua);
}

ME_LUA_BINDING(
    "engine:ui:UIRoot",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)