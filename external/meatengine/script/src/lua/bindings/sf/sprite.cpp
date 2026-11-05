#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>

static void bind(sol::state& lua) {
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
            [](sf::Sprite& s, me::Texture& tex, sol::optional<bool> resetRect) {
                s.setTexture(tex.res, resetRect.value_or(false));
            },
        "setTextureRect", &sf::Sprite::setTextureRect,
        "setColor", &sf::Sprite::setColor,
        "getTextureRect", &sf::Sprite::getTextureRect,
        "getColor", &sf::Sprite::getColor,
        "getLocalBounds", &sf::Sprite::getLocalBounds,
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

ME_LUA_BINDING(
    "engine:sf:Sprite",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)