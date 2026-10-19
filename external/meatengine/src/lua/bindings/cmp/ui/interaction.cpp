#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/Interaction.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::cmp::ui::Interaction>("Interaction", {
        { "None", me::cmp::ui::Interaction::None },
        { "Hovered", me::cmp::ui::Interaction::Hovered },
        { "Pressed", me::cmp::ui::Interaction::Pressed },
    });

    me::lua::register_component<me::cmp::ui::Interaction>("Interaction", lua);
}

ME_LUA_BINDING(
    "engine:ui:Interaction",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)