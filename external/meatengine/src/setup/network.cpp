#pragma once

#include <meatengine/net/Network.hpp>

#include <meatengine/registry_setup.hpp>
#include <meatengine/cmp/net/NetId.hpp>

namespace {
void add(entt::registry& r, entt::entity e) {
    me::net::Network::assign(
        r.get<me::cmp::net::NetId>(e).value,
        e
    );
}

void remove(entt::registry& r, entt::entity e) {
    me::net::Network::remove(e);
}

void install(entt::registry& reg) {
    reg.on_construct<me::cmp::net::NetId>().connect<&add>();
    reg.on_destroy<me::cmp::net::NetId>().connect<&remove>();
}
}

ME_REGISTER_REGISTRY_SETUP(
	"engine:network",
	[](entt::registry& r) { install(r); }
)