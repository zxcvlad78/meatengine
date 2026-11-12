#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/Interaction.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::ui::cmp::Interaction>("Interaction", {
        { "None", me::ui::cmp::Interaction::None },
        { "Hovered", me::ui::cmp::Interaction::Hovered },
        { "Pressed", me::ui::cmp::Interaction::Pressed },
    });

    me::lua::register_component<me::ui::cmp::Interaction>("Interaction", lua);
}

ME_LUA_BINDING(
    "engine:ui:Interaction",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)