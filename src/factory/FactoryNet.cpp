#include "FactoryRuntime.hpp"

#include <meatengine/net/Network.hpp>
#include <meatengine/net/Rpc.hpp>
#include <meatengine/scripting/ScriptingServer.hpp>

#include <cstdio>
#include <string>
#include <unordered_map>

namespace godlike::factory {

namespace {

bool server_side() { return role() == Role::Server; }

void send_world(std::uint32_t peer) {
    me::net::Rpc::invoke_to(peer, "factory:world", static_cast<std::uint32_t>(world().seed()));
    me::net::Rpc::invoke_to(peer, "factory:entities", world().encode_entities());
    me::net::Rpc::invoke_to(peer, "factory:inventory", world().encode_inventory(peer));
    me::net::Rpc::invoke_to(peer, "factory:research", world().encode_research());
}

void notify_peer(std::uint32_t peer, const std::string& text) {
    me::net::Rpc::invoke_to(peer, "factory:notify", text);
}

}

void bind_network() {
    me::net::Network::set_batching(true);

    using me::net::Rpc;
    using me::net::BinaryReader;
    using me::net::RpcContext;

    Rpc::bind("factory:sync_request", [](RpcContext& ctx, BinaryReader&) {
        if (!server_side()) return;
        send_world(ctx.sender);
    });

    Rpc::bind("factory:world", [](RpcContext&, BinaryReader& r) {
        if (role() != Role::Client) return;
        std::uint32_t seed = 0;
        if (!r.read(seed)) return;
        if (world().ready() && world().seed() == seed) return;
        world().generate(seed);
    });

    Rpc::bind("factory:entities", [](RpcContext&, BinaryReader& r) {
        if (role() != Role::Client) return;
        std::string data;
        if (!r.read(data)) return;
        world().decode_entities(data);
    });

    Rpc::bind("factory:inventory", [](RpcContext&, BinaryReader& r) {
        if (role() != Role::Client) return;
        std::string data;
        if (!r.read(data)) return;
        world().decode_inventory(local_peer(), data);
    });

    Rpc::bind("factory:research", [](RpcContext&, BinaryReader& r) {
        if (role() != Role::Client) return;
        std::string data;
        if (!r.read(data)) return;
        world().decode_research(data);
    });

    Rpc::bind("factory:notify", [](RpcContext&, BinaryReader& r) {
        if (role() != Role::Client) return;
        std::string text;
        if (!r.read(text)) return;
        sol::state& lua = me::ScriptingServer::lua();
        sol::table payload = lua.create_table();
        payload["message"] = text;
        dispatch_event("on_notice", payload);
    });

    Rpc::bind("factory:build", [](RpcContext& ctx, BinaryReader& r) {
        if (!server_side()) return;
        std::string name;
        std::int32_t x = 0;
        std::int32_t y = 0;
        std::int32_t dir = 2;
        if (!r.read(name) || !r.read(x) || !r.read(y) || !r.read(dir)) return;
        std::string error;
        if (!world().build(ctx.sender, x, y, registry().entity_id(name), dir, &error))
            notify_peer(ctx.sender, error);
    });

    Rpc::bind("factory:remove", [](RpcContext& ctx, BinaryReader& r) {
        if (!server_side()) return;
        std::int32_t x = 0;
        std::int32_t y = 0;
        if (!r.read(x) || !r.read(y)) return;
        std::string error;
        if (!world().remove(x, y, &error)) notify_peer(ctx.sender, error);
    });

    Rpc::bind("factory:mine", [](RpcContext& ctx, BinaryReader& r) {
        if (!server_side()) return;
        std::int32_t x = 0;
        std::int32_t y = 0;
        if (!r.read(x) || !r.read(y)) return;
        std::string error;
        if (!world().mine(x, y, &error)) notify_peer(ctx.sender, error);
    });

    Rpc::bind("factory:rotate", [](RpcContext& ctx, BinaryReader& r) {
        if (!server_side()) return;
        std::int32_t x = 0;
        std::int32_t y = 0;
        if (!r.read(x) || !r.read(y)) return;
        std::string error;
        if (!world().rotate(x, y, &error)) notify_peer(ctx.sender, error);
    });

    Rpc::bind("factory:craft", [](RpcContext& ctx, BinaryReader& r) {
        if (!server_side()) return;
        std::string recipe;
        if (!r.read(recipe)) return;
        std::string error;
        if (!world().craft(ctx.sender, registry().recipe_id(recipe), &error))
            notify_peer(ctx.sender, error);
    });

    Rpc::bind("factory:set_recipe", [](RpcContext& ctx, BinaryReader& r) {
        if (!server_side()) return;
        std::int32_t x = 0;
        std::int32_t y = 0;
        std::string recipe;
        if (!r.read(x) || !r.read(y) || !r.read(recipe)) return;
        std::string error;
        if (!world().set_recipe(x, y, registry().recipe_id(recipe), &error))
            notify_peer(ctx.sender, error);
    });

    Rpc::bind("factory:give", [](RpcContext& ctx, BinaryReader& r) {
        if (!server_side()) return;
        std::string item;
        std::int32_t count = 0;
        if (!r.read(item) || !r.read(count)) return;
        world().give_item(ctx.sender, registry().item_id(item), count);
    });
}

void update_network(float dt) {
    if (!server_side()) return;

    const std::string entities = world().encode_entities();
    const std::string research = world().encode_research();

    for (auto& [peer, state] : peer_states()) {
        if (state.entities != entities) {
            state.entities = entities;
            me::net::Rpc::invoke_to(peer, "factory:entities", entities);
        }

        const std::string inventory = world().encode_inventory(peer);
        if (state.inventory != inventory) {
            state.inventory = inventory;
            me::net::Rpc::invoke_to(peer, "factory:inventory", inventory);
        }

        if (state.research != research) {
            state.research = research;
            me::net::Rpc::invoke_to(peer, "factory:research", research);
        }
    }
}

}
