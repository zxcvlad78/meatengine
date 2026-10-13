#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/UIRoot.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::UIRoot>("UIRoot", sol::no_constructor);
    me::lua::register_tag<me::cmp::ui::UIRoot>("UIRoot", lua);
}

ME_LUA_BINDING(
    "engine:ui:UIRoot",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)