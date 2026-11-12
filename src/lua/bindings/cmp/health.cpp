#include <string>
#include <meatengine/scripting/LuaBinder.hpp>

#include <godlike/cmp/Health.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<godlike::cmp::Health>("Health",
        sol::constructors<godlike::cmp::Health()>(),
        "points", &godlike::cmp::Health::points
    );

    me::lua::register_component<godlike::cmp::Health>("Health", lua);
}

ME_LUA_BINDING(
    "godlike:cmp:Health",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)
