#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/Val.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::cmp::ui::Val::Kind>("ValKind", {
        { "Auto",    me::cmp::ui::Val::Kind::Auto },
        { "Px",      me::cmp::ui::Val::Kind::Px },
        { "Percent", me::cmp::ui::Val::Kind::Percent },
    });

    auto ut = lua.new_usertype<me::cmp::ui::Val>("Val",
        sol::constructors<me::cmp::ui::Val()>(),
        "kind",    &me::cmp::ui::Val::kind,
        "value",   &me::cmp::ui::Val::value,
        "is_auto", &me::cmp::ui::Val::is_auto
    );

    ut.set_function("Auto", []() {
        return me::cmp::ui::Val::Auto();
    });
    ut.set_function("Px", [](float v) {
        return me::cmp::ui::Val::Px(v);
    });
    ut.set_function("Percent", [](float v) {
        return me::cmp::ui::Val::Percent(v);
    });
}

ME_LUA_BINDING(
    "engine:ui:Val",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)