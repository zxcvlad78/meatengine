#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/ZIndex.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ZIndex>("ZIndex",
        sol::constructors<me::cmp::ZIndex()>(),
        "value", &me::cmp::ZIndex::value
    );
    
    me::lua::register_component<me::cmp::ZIndex>("ZIndex", lua);
}

ME_LUA_BINDING(
    "engine:cmp:ZIndex",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)