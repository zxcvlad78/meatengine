#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/UiRect.hpp>

static void bind(sol::state& lua) {
    auto ut = lua.new_usertype<me::cmp::ui::UiRect>("UiRect",
        sol::constructors<me::cmp::ui::UiRect()>(),
        "top", &me::cmp::ui::UiRect::top,
        "right", &me::cmp::ui::UiRect::right,
        "bottom", &me::cmp::ui::UiRect::bottom,
        "left", &me::cmp::ui::UiRect::left
    );

    ut.set_function("all", &me::cmp::ui::UiRect::all);
    ut.set_function("axes", [](me::cmp::ui::Val vert, me::cmp::ui::Val horiz) {
        return me::cmp::ui::UiRect::axes(vert, horiz);
    });
}

ME_LUA_BINDING(
    "engine:ui:UiRect",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)