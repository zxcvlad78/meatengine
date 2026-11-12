#include <string>
#include <meatengine/scripting/LuaBinder.hpp>

#include <meatengine/abstract/cmp/ZIndex.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::abstract::cmp::ZIndex>("ZIndex",
        sol::constructors<me::abstract::cmp::ZIndex()>(),
        "value", &me::abstract::cmp::ZIndex::value
    );
    
    me::lua::register_component<me::abstract::cmp::ZIndex>("ZIndex", lua);
}

ME_LUA_BINDING(
    "engine:cmp:ZIndex",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)