#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <meatengine/net/Network.hpp>
#include <meatengine/net/Rpc.hpp>

static void run() {
    if (me::net::Network::is_server() || me::net::Network::is_client())
        me::net::Rpc::update(me::MainLoop::get_dt());
}

ME_REGISTER_SYSTEM(
    "engine:network:rpc:update",
    me::SystemPhase::PostRender,
    std::numeric_limits<int>::min() + 2,
    run
)