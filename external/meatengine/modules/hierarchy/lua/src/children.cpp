#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/hierarchy/cmp/Children.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::hierarchy::cmp::Children>("Children", sol::no_constructor,
        "entities", &me::hierarchy::cmp::Children::entities
    );

    me::lua::register_component<me::hierarchy::cmp::Children>("Children", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Children",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)