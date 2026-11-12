#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/UiRect.hpp>

static void bind(sol::state& lua) {
    auto ut = lua.new_usertype<me::ui::cmp::UiRect>("UiRect",
        sol::constructors<me::ui::cmp::UiRect()>(),
        "top", &me::ui::cmp::UiRect::top,
        "right", &me::ui::cmp::UiRect::right,
        "bottom", &me::ui::cmp::UiRect::bottom,
        "left", &me::ui::cmp::UiRect::left
    );

    ut.set_function("all", &me::ui::cmp::UiRect::all);
    ut.set_function("axes", [](me::ui::cmp::Val vert, me::ui::cmp::Val horiz) {
        return me::ui::cmp::UiRect::axes(vert, horiz);
    });
}

ME_LUA_BINDING(
    "engine:ui:UiRect",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)