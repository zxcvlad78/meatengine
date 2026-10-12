#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::SoundBuffer>("SoundBuffer",
        sol::no_constructor,
        "duration", sol::property(
            [](me::SoundBuffer& sb) { return sb.res.getDuration().asSeconds(); }
        )
    );

    sol::table rl = lua["ResourceLoader"];
    me::lua::register_resource<me::SoundBuffer>(rl, "SoundBuffer");
}

ME_LUA_BINDING(
    "engine:SoundBuffer",
    static_cast<unsigned>(me::lua::BindPriority::Resource),
    &bind
)