#pragma once

#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/net/Network.hpp>

static void run() {
    if (auto* s = me::net::Network::server()) s->Update();
    else if (auto* c = me::net::Network::client()) c->Update();

    me::net::Network::update_stats();
}


ME_REGISTER_SYSTEM(
    "engine:network:update",
    me::SystemPhase::Input,
    std::numeric_limits<int>::min(),
    run
)
