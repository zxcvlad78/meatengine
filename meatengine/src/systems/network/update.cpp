#pragma once

#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <meatengine/meatnet/Network.hpp>

void update() {
    auto& reg = me::MainLoop::get_registry();
    float dt = me::MainLoop::get_fixed_dt();

    auto* s = me::net::Network::server();
    if (s != nullptr)
        s->Update();

    auto* c = me::net::Network::client();
    if (c != nullptr)
        c->Update();
}


ME_REGISTER_SYSTEM(
    "engine:network:update",
    me::SystemPhase::FixedUpdate,
    std::numeric_limits<int>::min(),
    update
)
