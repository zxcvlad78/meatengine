#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <SFML/System/Vector2.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<sf::Vector2f>("Vector2f",
        sol::constructors<sf::Vector2f(), sf::Vector2f(float, float)>(),
        "x", &sf::Vector2f::x,
        "y", &sf::Vector2f::y
    );
}


ME_LUA_BINDING(
    "engine:sf:Vector2f",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)