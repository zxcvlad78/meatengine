#pragma once

#include <meatengine/net/Network.hpp>

#include <meatengine/ecs/registry_setup.hpp>
#include <meatengine/net/cmp/NetId.hpp>

namespace {
void add(entt::registry& r, entt::entity e) {
    me::net::Network::assign(
        r.get<me::net::cmp::NetId>(e).value,
        e
    );
}

void remove(entt::registry& r, entt::entity e) {
    me::net::Network::remove(e);
}

void install(entt::registry& reg) {
    reg.on_construct<me::net::cmp::NetId>().connect<&add>();
    reg.on_destroy<me::net::cmp::NetId>().connect<&remove>();
}
}

ME_REGISTER_REGISTRY_SETUP(
	"engine:network",
	[](entt::registry& r) { install(r); }
)