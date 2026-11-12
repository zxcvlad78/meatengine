#include <meatengine/app/MainLoop.hpp>
#include <meatengine/assets/ResourceLoader.hpp>
#include <meatengine/assets/Resources.hpp>
#include <meatengine/debug/Console.hpp>

namespace {

    void on_init(sf::RenderWindow&) {
        auto font = me::ResourceLoader::get_default<me::Font>();
        if (!font) return;
        me::debug::Console::get_instance().init(font->res, 16);
    }

    void on_update(sf::RenderWindow& window, float dt) {
        me::debug::Console::get_instance().update(window, dt);
    }

    void on_render(sf::RenderWindow& window) {
        me::debug::Console::get_instance().render(window);
    }

    void on_event(const sf::Event& event, sf::RenderWindow& window) {
        me::debug::Console::get_instance().handle_event(event, window);
    }

    struct DebugConsoleHooks {
        DebugConsoleHooks() {
            me::MainLoop::add_on_init(&on_init);
            me::MainLoop::add_on_update(&on_update);
            me::MainLoop::add_on_render(&on_render);
            me::MainLoop::add_on_event(&on_event);
        }
    };

    DebugConsoleHooks g_debug_console_hooks;
}
