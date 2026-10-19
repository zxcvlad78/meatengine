#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <SFML/System/Vector2.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<sf::Vector2u>("Vector2u",
        sol::constructors<sf::Vector2u(), sf::Vector2u(unsigned, unsigned)>(),
        "x", &sf::Vector2u::x,
        "y", &sf::Vector2u::y
    );
}

ME_LUA_BINDING(
    "engine:sf:Vector2u",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)