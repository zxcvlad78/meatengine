#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <SFML/System/Vector2.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<sf::Vector2i>("Vector2i",
        sol::constructors<sf::Vector2i(), sf::Vector2i(int, int)>(),
        "x", &sf::Vector2i::x,
        "y", &sf::Vector2i::y
    );
}

ME_LUA_BINDING(
    "engine:sf:Vector2i",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)