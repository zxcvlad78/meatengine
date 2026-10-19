#pragma once

#include <meatengine/SystemRegistry.hpp>
#include <meatengine/net/Network.hpp>

void run() {
    if (auto* s = me::net::Network::server()) s->Update();
    else if (auto* c = me::net::Network::client()) c->Update();
}


ME_REGISTER_SYSTEM(
    "engine:network:update",
    me::SystemPhase::Input,
    std::numeric_limits<int>::min(),
    run
)
