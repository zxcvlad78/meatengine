#pragma once

#include <meatengine/GameState.hpp>
#include <sol/sol.hpp>
#include <string>
#include <iostream>

namespace me {

class ScriptedState : public GameState {
public:
    explicit ScriptedState(std::string name) : _name(std::move(name)) {}

    void set_on_enter(sol::protected_function fn) { _on_enter = std::move(fn); }
    void set_on_exit(sol::protected_function fn) { _on_exit = std::move(fn); }
    void set_handle_event(sol::protected_function fn) { _handle_event = std::move(fn); }

    const std::string& name() const { return _name; }

    void on_enter(sf::RenderWindow& w, entt::registry& r) override {
        _call(_on_enter, "on_enter", w, r);
    }
    void on_exit(sf::RenderWindow& w, entt::registry& r) override {
        _call(_on_exit, "on_exit", w, r);
    }
    void handle_event(sf::RenderWindow& w, entt::registry& r, const sf::Event& e) override {
        _call(_handle_event, "handle_event", w, r, e);
    }

private:
    std::string _name;

    sol::protected_function _on_enter;
    sol::protected_function _on_exit;
    sol::protected_function _handle_event;

    template<typename... Args>
    void _call(sol::protected_function& fn, const char* method, Args&&... args) {
        if (!fn.valid()) return;

        auto res = fn(std::forward<Args>(args)...);
        if (!res.valid()) {
            sol::error err = res;
            std::cerr << "[ScriptedState] "
                << _name << "] " << method
                << " failed: " << err.what() << "\n";
            fn = sol::protected_function{};
        }
    }
};

} // namespace me