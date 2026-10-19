#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/GlobalZIndex.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::GlobalZIndex>("GlobalZIndex",
        sol::constructors<me::cmp::GlobalZIndex()>(),
        "value", &me::cmp::GlobalZIndex::value
    );

    me::lua::register_component<me::cmp::GlobalZIndex>("GlobalZIndex", lua);
}

ME_LUA_BINDING(
    "engine:cmp:GlobalZIndex",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)