#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <SFML/Graphics/RectangleShape.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<sf::RectangleShape>("RectangleShape",
        sol::constructors<sf::RectangleShape(), sf::RectangleShape(sf::Vector2f)>(),
        "position", sol::property(
            [](sf::RectangleShape& s) { return s.getPosition(); },
            [](sf::RectangleShape& s, sf::Vector2f p) { s.setPosition(p); }
        ),
        "size", sol::property(
            [](sf::RectangleShape& s) { return s.getSize(); },
            [](sf::RectangleShape& s, sf::Vector2f sz) { s.setSize(sz); }
        ),
        "fill_color", sol::property(
            [](sf::RectangleShape& s) { return s.getFillColor(); },
            [](sf::RectangleShape& s, sf::Color c) { s.setFillColor(c); }
        ),
        "outline_color", sol::property(
            [](sf::RectangleShape& s) { return s.getOutlineColor(); },
            [](sf::RectangleShape& s, sf::Color c) { s.setOutlineColor(c); }
        ),
        "outline_thickness", sol::property(
            [](sf::RectangleShape& s) { return s.getOutlineThickness(); },
            [](sf::RectangleShape& s, float t) { s.setOutlineThickness(t); }
        )
    );
}


ME_LUA_BINDING(
    "engine:sf:RectangleShape",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)