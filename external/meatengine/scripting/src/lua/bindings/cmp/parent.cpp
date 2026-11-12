#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/hierarchy/cmp/Parent.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::hierarchy::cmp::Parent>("Parent",
        sol::constructors<me::hierarchy::cmp::Parent()>(),
        "entity", &me::hierarchy::cmp::Parent::entity
    );

    me::lua::register_component<me::hierarchy::cmp::Parent>("Parent", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Parent",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)