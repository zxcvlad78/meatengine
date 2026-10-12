#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/ui/Interactable.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::Interactable>("Interactable",
        sol::constructors<me::cmp::ui::Interactable()>()
    );

    me::lua::register_component<me::cmp::ui::Interactable>("Interactable", lua);
}

ME_LUA_BINDING(
    "engine:cmp:ui:Interactable",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)