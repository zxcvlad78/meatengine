#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <godlike/cmp/MoveSpeed.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<godlike::cmp::MoveSpeed>("MoveSpeed",
        sol::constructors<godlike::cmp::MoveSpeed()>(),
        "value", &godlike::cmp::MoveSpeed::value
    );

    me::lua::register_component<godlike::cmp::MoveSpeed>("MoveSpeed", lua);
}

ME_LUA_BINDING(
    "godlike:cmp:MoveSpeed",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)
