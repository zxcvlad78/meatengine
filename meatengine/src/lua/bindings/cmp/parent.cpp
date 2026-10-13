#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/Parent.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::Parent>("Parent",
        sol::constructors<me::cmp::Parent()>(),
        "entity", &me::cmp::Parent::entity
    );

    me::lua::register_component<me::cmp::Parent>("Parent", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Parent",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)