#include <string>
#include <meatengine/scripting/LuaBinder.hpp>

#include <meatengine/app/MainLoop.hpp>
#include <meatengine/graphics/cmp/Camera.hpp>

static void bind(sol::state& lua) {
    auto usertype = lua.new_usertype<me::graphics::cmp::Camera>("Camera",
        sol::constructors<me::graphics::cmp::Camera()>(),
        "zoom", &me::graphics::cmp::Camera::zoom,
        "smooth", &me::graphics::cmp::Camera::smooth
    );

    usertype.set_function("is_current", &me::graphics::cmp::Camera::is_current);
    usertype.set_function("set_current", &me::graphics::cmp::Camera::set_current);
    usertype.set_function("make_current", &me::graphics::cmp::Camera::make_current);
    usertype.set_function("get_current", &me::graphics::cmp::Camera::get_current);

    usertype.set_function("view_center", [](me::graphics::cmp::Camera& cam) -> sf::Vector2f {
        return cam.view.getCenter();
    });

    usertype.set_function("screen_to_world", [](me::graphics::cmp::Camera& cam, sf::Vector2f screen) -> sf::Vector2f {
        auto& window = me::MainLoop::get_window();
        const sf::Vector2f window_size = static_cast<sf::Vector2f>(window.getSize());
        if (window_size.x <= 0.f || window_size.y <= 0.f) return screen;

        const sf::Vector2f center = cam.view.getCenter();
        const sf::Vector2f view_size = cam.view.getSize();

        return sf::Vector2f{
            center.x + (screen.x - window_size.x * 0.5f) * view_size.x / window_size.x,
            center.y + (screen.y - window_size.y * 0.5f) * view_size.y / window_size.y
        };
    });

    me::lua::register_component<me::graphics::cmp::Camera>("Camera", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Camera",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)