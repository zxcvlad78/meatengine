#include <meatengine/app/MainLoop.hpp>
#include <meatengine/modding/ModLoader.hpp>

namespace {

    struct ModLoaderHooks {
        ModLoaderHooks() {
            me::MainLoop::add_on_init([](sf::RenderWindow&) {
                me::ModLoader::load();
            });
        }
    };

    ModLoaderHooks g_mod_loader_hooks;
}
