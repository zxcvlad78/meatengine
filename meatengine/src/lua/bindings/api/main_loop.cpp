#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/MainLoop.hpp>

static void bind(sol::state& lua) {
    sol::table table = lua.create_table();
    lua["MainLoop"] = table;

    table.set_function("get_registry",
        []() -> entt::registry& {
            return me::MainLoop::get_registry();
        });
    
    table.set_function("get_window",
        []() -> sf::RenderWindow& {
            return me::MainLoop::get_window();
        });

    table.set_function("get_window_title",
        []() -> std::string {
            return me::MainLoop::get_window_title();
        });

    table.set_function("set_window_title",
        [](const std::string& s) -> void {
            me::MainLoop::set_window_title(s);
        });

    table.set_function("get_framerate_limit",
        []() -> float {
            return me::MainLoop::get_framerate_limit();
        });

    table.set_function("set_framerate_limit",
        [](float v) -> void {
            me::MainLoop::set_framerate_limit(v);
        });
}

ME_LUA_BINDING(
    "engine:api:MainLoop",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)