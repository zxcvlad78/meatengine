#include <meatengine/app/MainLoop.hpp>
#include <meatengine/net/Utils.hpp>

#include <cstdio>

namespace {

    struct NetLifecycleHooks {
        NetLifecycleHooks() {
            me::MainLoop::add_on_init([](sf::RenderWindow&) {
                if (!me::net::InitNetwork())
                    std::printf("[Network] failed to initialize\n");
            });
            me::MainLoop::add_on_shutdown([] {
                me::net::ShutdownNetwork();
            });
        }
    };

    NetLifecycleHooks g_net_hooks;
}
