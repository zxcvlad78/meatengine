#include <string>
#include <meatengine/lua/LuaBinder.hpp>

static void bind(sol::state& lua) {
    
}

ME_LUA_BINDING(
    "namespace:name",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)
