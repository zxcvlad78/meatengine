#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::StyleBox>("StyleBox",
        sol::no_constructor,

        "get_color", [](me::StyleBox& sb, const std::string& k, sol::optional<sf::Color> def) {
            return sb.get_value<sf::Color>(k, def.value_or(sf::Color::White));
        },
        "get_float", [](me::StyleBox& sb, const std::string& k, sol::optional<float> def) {
            return sb.get_value<float>(k, def.value_or(0.f));
        },
        "get_int", [](me::StyleBox& sb, const std::string& k, sol::optional<int> def) {
            return sb.get_value<int>(k, def.value_or(0));
        },
        "get_bool", [](me::StyleBox& sb, const std::string& k, sol::optional<bool> def) {
            return sb.get_value<bool>(k, def.value_or(false));
        },
        "get_string", [](me::StyleBox& sb, const std::string& k, sol::optional<std::string> def) {
            return sb.get_value<std::string>(k, def.value_or(""));
        },
        "set_color", [](me::StyleBox& sb, const std::string& k, sf::Color v) {
            sb.set_value<sf::Color>(k, v);
        },
        "set_float", [](me::StyleBox& sb, const std::string& k, float v) {
            sb.set_value<float>(k, v);
        },
        "set_int", [](me::StyleBox& sb, const std::string& k, int v) {
            sb.set_value<int>(k, v);
        },
        "set_bool", [](me::StyleBox& sb, const std::string& k, bool v) {
            sb.set_value<bool>(k, v);
        },
        "set_string", [](me::StyleBox& sb, const std::string& k, const std::string& v) {
            sb.set_value<std::string>(k, v);
        }
    );
    
    sol::table rl = lua["ResourceLoader"];
    me::lua::register_resource<me::StyleBox>(rl, "StyleBox");
}

ME_LUA_BINDING(
    "engine:StyleBox",
    static_cast<unsigned>(me::lua::BindPriority::Resource),
    &bind
)