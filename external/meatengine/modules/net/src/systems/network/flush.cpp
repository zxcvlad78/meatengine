#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/net/Network.hpp>

static void run() {
    if (!me::net::Network::is_server() && !me::net::Network::is_client()) return;
    me::net::Network::flush();
}

ME_REGISTER_SYSTEM(
    "engine:network:flush",
    me::SystemPhase::PostRender,
    std::numeric_limits<int>::max(),
    run
)
