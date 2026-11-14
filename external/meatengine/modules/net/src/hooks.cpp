#include <meatengine/app/MainLoop.hpp>
#include <meatengine/ecs/PackedEntity.hpp>
#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/net/Network.hpp>
#include <meatengine/net/Rpc.hpp>
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
                me::net::Rpc::clear_handlers();
                me::net::Rpc::clear();
                me::net::Network::set_on_peer_connected(me::net::Network::PeerEventCallback{});
                me::net::Network::set_on_peer_disconnected(me::net::Network::PeerEventCallback{});
                me::PackedEntity::clear();
                me::SystemRegistry::clear();
                me::net::ShutdownNetwork();
            });
        }
    };

    NetLifecycleHooks g_net_hooks;
}
