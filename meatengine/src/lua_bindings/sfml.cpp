#include "meatengine/lua_bindings/components.hpp"
#include "meatengine/lua_bindings/common.hpp"
#include "meatengine/meatengine.hpp"

#include <string>

void me::lua_bindings::init_sfml(sol::state& lua) {
    lua.new_usertype<sf::Vector2f>("Vector2f",
        sol::constructors<sf::Vector2f(), sf::Vector2f(float, float)>(),
        "x", &sf::Vector2f::x,
        "y", &sf::Vector2f::y
    );

    lua.new_usertype<sf::Vector2u>("Vector2u",
        sol::constructors<sf::Vector2u(), sf::Vector2u(unsigned, unsigned)>(),
        "x", &sf::Vector2u::x,
        "y", &sf::Vector2u::y
    );

    lua.new_usertype<sf::Vector2i>("Vector2i",
        sol::constructors<sf::Vector2i(), sf::Vector2i(int, int)>(),
        "x", &sf::Vector2i::x,
        "y", &sf::Vector2i::y
    );

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
        "size",     &sf::IntRect::size
    );

    lua.new_usertype<sf::FloatRect>("FloatRect",
        sol::no_constructor,
        sol::meta_function::construct, sol::factories(
            []() { return sf::FloatRect(); },
            [](sf::Vector2f pos, sf::Vector2f size) {
                return sf::FloatRect(pos, size);
            },
            [](float left, float top, float width, float height) {
                return sf::FloatRect({left, top}, {width, height});
            }
        ),
        "position", &sf::FloatRect::position,
        "size",     &sf::FloatRect::size
    );

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


    lua.new_usertype<sf::Sprite>("sf_Sprite",
        sol::no_constructor,
        sol::meta_function::construct, sol::factories(
            [](me::Texture& tex) {
                return sf::Sprite(tex.res);
            },
            [](me::Texture& tex, const sf::IntRect& rect) {
                return sf::Sprite(tex.res, rect);
            },
            [](me::Texture& tex, int left, int top, int width, int height) {
                return sf::Sprite(tex.res, sf::IntRect({left, top}, {width, height}));
            }
        ),

        "setTexture",
            [](sf::Sprite& sprite, me::Texture& tex, sol::optional<bool> resetRect) {
                sprite.setTexture(tex.res, resetRect.value_or(false));
            },
        "setTextureRect",  &sf::Sprite::setTextureRect,
        "setColor",        &sf::Sprite::setColor,
        "getTextureRect",  &sf::Sprite::getTextureRect,
        "getColor",        &sf::Sprite::getColor,
        "getLocalBounds",  &sf::Sprite::getLocalBounds,
        "getGlobalBounds", &sf::Sprite::getGlobalBounds,

        "setPosition", sol::overload(
            [](sf::Sprite& s, float x, float y) { s.setPosition({x, y}); },
            [](sf::Sprite& s, const sf::Vector2f& pos) { s.setPosition(pos); }
        ),
        "setRotation", sol::overload(
            [](sf::Sprite& s, float angle) { s.setRotation(sf::degrees(angle)); },
            [](sf::Sprite& s, sf::Angle angle) { s.setRotation(angle); }
        ),
        "setScale", sol::overload(
            [](sf::Sprite& s, float x, float y) { s.setScale({x, y}); },
            [](sf::Sprite& s, const sf::Vector2f& scale) { s.setScale(scale); }
        ),
        "setOrigin", sol::overload(
            [](sf::Sprite& s, float x, float y) { s.setOrigin({x, y}); },
            [](sf::Sprite& s, const sf::Vector2f& origin) { s.setOrigin(origin); }
        ),
        "getPosition", &sf::Sprite::getPosition,
        "getRotation", [](sf::Sprite& s) { return s.getRotation().asDegrees(); },
        "getScale",    &sf::Sprite::getScale,
        "getOrigin",   &sf::Sprite::getOrigin,
        "move", sol::overload(
            [](sf::Sprite& s, float x, float y) { s.move({x, y}); },
            [](sf::Sprite& s, const sf::Vector2f& offset) { s.move(offset); }
        ),
        "rotate", [](sf::Sprite& s, float angle) { s.rotate(sf::degrees(angle)); },
        "scale", sol::overload(
            [](sf::Sprite& s, float x, float y) { s.scale({x, y}); },
            [](sf::Sprite& s, const sf::Vector2f& factors) { s.scale(factors); }
        ),
        "getTransform",        &sf::Sprite::getTransform,
        "getInverseTransform", &sf::Sprite::getInverseTransform
    );
}