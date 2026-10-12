#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <SFML/Graphics/Color.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<sf::Color>("Color",
        sol::constructors<
            sf::Color(),
            sf::Color(std::uint8_t, std::uint8_t, std::uint8_t),
            sf::Color(std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t)
        >(),
        "r", &sf::Color::r,
        "g", &sf::Color::g,
        "b", &sf::Color::b,
        "a", &sf::Color::a
    );
}


ME_LUA_BINDING(
    "engine:sf:Vector2i",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)