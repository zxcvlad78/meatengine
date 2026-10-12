#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <SFML/Graphics/Rect.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<sf::FloatRect>("FloatRect",
        sol::no_constructor,
        sol::meta_function::construct, sol::factories(
            []() {
                return sf::FloatRect();
            },
            [](sf::Vector2f pos, sf::Vector2f size) {
                return sf::FloatRect(pos, size);
            },
            [](float left, float top, float width, float height) {
                return sf::FloatRect({left, top}, {width, height});
            }
        ),
        "position", &sf::FloatRect::position,
        "size", &sf::FloatRect::size
    );
}


ME_LUA_BINDING(
    "engine:sf:FloatRect",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)