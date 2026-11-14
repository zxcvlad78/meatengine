#include <string>
#include <unordered_map>

#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/app/ctx/InputState.hpp>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace {

    me::app::ctx::InputState* input_state() {
        auto& r = me::MainLoop::get_registry();
        if (!r.ctx().contains<me::app::ctx::InputState>()) return nullptr;
        return &r.ctx().get<me::app::ctx::InputState>();
    }

    const std::unordered_map<std::string, sf::Keyboard::Key>& key_map() {
        static const std::unordered_map<std::string, sf::Keyboard::Key> map = {
            {"a", sf::Keyboard::Key::A},
            {"b", sf::Keyboard::Key::B},
            {"c", sf::Keyboard::Key::C},
            {"d", sf::Keyboard::Key::D},
            {"e", sf::Keyboard::Key::E},
            {"f", sf::Keyboard::Key::F},
            {"g", sf::Keyboard::Key::G},
            {"h", sf::Keyboard::Key::H},
            {"i", sf::Keyboard::Key::I},
            {"j", sf::Keyboard::Key::J},
            {"k", sf::Keyboard::Key::K},
            {"l", sf::Keyboard::Key::L},
            {"m", sf::Keyboard::Key::M},
            {"n", sf::Keyboard::Key::N},
            {"o", sf::Keyboard::Key::O},
            {"p", sf::Keyboard::Key::P},
            {"q", sf::Keyboard::Key::Q},
            {"r", sf::Keyboard::Key::R},
            {"s", sf::Keyboard::Key::S},
            {"t", sf::Keyboard::Key::T},
            {"u", sf::Keyboard::Key::U},
            {"v", sf::Keyboard::Key::V},
            {"w", sf::Keyboard::Key::W},
            {"x", sf::Keyboard::Key::X},
            {"y", sf::Keyboard::Key::Y},
            {"z", sf::Keyboard::Key::Z},
            {"0", sf::Keyboard::Key::Num0},
            {"1", sf::Keyboard::Key::Num1},
            {"2", sf::Keyboard::Key::Num2},
            {"3", sf::Keyboard::Key::Num3},
            {"4", sf::Keyboard::Key::Num4},
            {"5", sf::Keyboard::Key::Num5},
            {"6", sf::Keyboard::Key::Num6},
            {"7", sf::Keyboard::Key::Num7},
            {"8", sf::Keyboard::Key::Num8},
            {"9", sf::Keyboard::Key::Num9},
            {"escape", sf::Keyboard::Key::Escape},
            {"tab", sf::Keyboard::Key::Tab},
            {"space", sf::Keyboard::Key::Space},
            {"enter", sf::Keyboard::Key::Enter},
            {"backspace", sf::Keyboard::Key::Backspace},
            {"left", sf::Keyboard::Key::Left},
            {"right", sf::Keyboard::Key::Right},
            {"up", sf::Keyboard::Key::Up},
            {"down", sf::Keyboard::Key::Down},
            {"lshift", sf::Keyboard::Key::LShift},
            {"lctrl", sf::Keyboard::Key::LControl},
            {"minus", sf::Keyboard::Key::Hyphen},
            {"equal", sf::Keyboard::Key::Equal},
            {"lbracket", sf::Keyboard::Key::LBracket},
            {"rbracket", sf::Keyboard::Key::RBracket},
        };
        return map;
    }

}

static void bind(sol::state& lua) {
    sol::table input = lua.create_table();
    lua["Input"] = input;

    input.set_function("mouse_screen", []() -> sf::Vector2f {
        return static_cast<sf::Vector2f>(sf::Mouse::getPosition(me::MainLoop::get_window()));
    });

    input.set_function("mouse_down", []() -> bool {
        auto* in = input_state();
        return in ? in->mouse_down : false;
    });

    input.set_function("mouse_just_pressed", []() -> bool {
        auto* in = input_state();
        return in ? in->mouse_just_pressed : false;
    });

    input.set_function("mouse_just_released", []() -> bool {
        auto* in = input_state();
        return in ? in->mouse_just_released : false;
    });

    input.set_function("mouse_right_down", []() -> bool {
        return sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
    });

    input.set_function("key_pressed", [](const std::string& name) -> bool {
        auto it = key_map().find(name);
        if (it == key_map().end()) return false;
        return sf::Keyboard::isKeyPressed(it->second);
    });

    input.set_function("window_size", []() -> sf::Vector2f {
        return static_cast<sf::Vector2f>(me::MainLoop::get_window().getSize());
    });

    input.set_function("focused", []() -> bool {
        return me::MainLoop::get_window().hasFocus();
    });
}

ME_LUA_BINDING(
    "engine:api:Input",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)
