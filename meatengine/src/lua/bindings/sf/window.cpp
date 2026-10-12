#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<sf::RenderWindow>("Window",
        sol::no_constructor,
        "size", [](sf::RenderWindow& w) { return w.getSize(); },
        "close", &sf::RenderWindow::close,
        "is_open", &sf::RenderWindow::isOpen,
        "set_title", [](sf::RenderWindow& w, const std::string& s) { w.setTitle(s); }
    );
}


ME_LUA_BINDING(
    "engine:sf:Window",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)