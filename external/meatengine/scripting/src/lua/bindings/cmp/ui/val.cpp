#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/Val.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::ui::cmp::Val::Kind>("ValKind", {
        { "Auto",    me::ui::cmp::Val::Kind::Auto },
        { "Px",      me::ui::cmp::Val::Kind::Px },
        { "Percent", me::ui::cmp::Val::Kind::Percent },
    });

    auto ut = lua.new_usertype<me::ui::cmp::Val>("Val",
        sol::constructors<me::ui::cmp::Val()>(),
        "kind",    &me::ui::cmp::Val::kind,
        "value",   &me::ui::cmp::Val::value,
        "is_auto", &me::ui::cmp::Val::is_auto
    );

    ut.set_function("Auto", []() {
        return me::ui::cmp::Val::Auto();
    });
    ut.set_function("Px", [](float v) {
        return me::ui::cmp::Val::Px(v);
    });
    ut.set_function("Percent", [](float v) {
        return me::ui::cmp::Val::Percent(v);
    });
}

ME_LUA_BINDING(
    "engine:ui:Val",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)