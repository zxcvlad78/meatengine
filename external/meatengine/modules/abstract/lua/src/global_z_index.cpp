#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/abstract/cmp/GlobalZIndex.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::abstract::cmp::GlobalZIndex>("GlobalZIndex",
        sol::constructors<me::abstract::cmp::GlobalZIndex()>(),
        "value", &me::abstract::cmp::GlobalZIndex::value
    );

    me::lua::register_component<me::abstract::cmp::GlobalZIndex>("GlobalZIndex", lua);
}

ME_LUA_BINDING(
    "engine:cmp:GlobalZIndex",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)