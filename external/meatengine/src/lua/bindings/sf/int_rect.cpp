#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <SFML/Graphics/Rect.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<sf::IntRect>("IntRect",
        sol::no_constructor,
        sol::meta_function::construct, sol::factories(
            []() { return sf::IntRect(); },
            [](sf::Vector2i pos, sf::Vector2i size) {
                return sf::IntRect(pos, size);
            },
            [](int left, int top, int width, int height) {
                return sf::IntRect({left, top}, {width, height});
            }
        ),
        "position", &sf::IntRect::position,
        "size", &sf::IntRect::size
    );
}

ME_LUA_BINDING(
    "engine:sf:IntRect",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)