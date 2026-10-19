#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::Texture>("Texture",
        sol::no_constructor,
        "size", sol::property(
            [](me::Texture& t) { return t.res.getSize(); }
        ),
        "smooth", sol::property(
            [](me::Texture& t) { return t.res.isSmooth(); },
            [](me::Texture& t, bool v) { t.res.setSmooth(v); }
        ),
        "repeated", sol::property(
            [](me::Texture& t) { return t.res.isRepeated(); },
            [](me::Texture& t, bool v) { t.res.setRepeated(v); }
        )
    );

    sol::table rl = lua["ResourceLoader"];
    me::lua::register_resource<me::Texture>(rl, "Texture");
}

ME_LUA_BINDING(
    "engine:Texture",
    static_cast<unsigned>(me::lua::BindPriority::Resource),
    &bind
)