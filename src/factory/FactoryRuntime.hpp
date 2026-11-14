#pragma once

#include "FactoryWorld.hpp"

#include <sol/sol.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace godlike::factory {

enum class Role {
    Single,
    Server,
    Client,
};

World& world();
Registry& registry();
Role& role();
std::uint32_t& local_peer();

using EventHandler = sol::protected_function;

struct PeerState {
    std::string entities;
    std::string inventory;
    std::string research;
};

std::unordered_map<std::uint32_t, PeerState>& peer_states();

std::unordered_map<std::string, EventHandler>& event_handlers();

void dispatch_event(const std::string& name, sol::table payload);

void bind_data_api(sol::state& lua);
void bind_control_api(sol::state& lua);
void bind_network();

void update_network(float dt);

}
