#include <meatengine/net/Network.hpp>
#include <meatengine/net/Rpc.hpp>

#include <cstdio>
#include <stdexcept>
#include <utility>

namespace me::net {

namespace {

struct Registry {
    std::unordered_map<RpcId, RpcHandler> handlers;
    std::unordered_map<RpcId, std::string> names;
};

Registry& registry() {
    static Registry r;
    return r;
}

std::unordered_map<std::uint32_t, PendingRpc>& pending_map() {
    static std::unordered_map<std::uint32_t, PendingRpc> p;
    return p;
}

std::uint64_t g_unknown_calls = 0;

}

std::unordered_map<RpcId, RpcHandler>& Rpc::handlers() { return registry().handlers; }
std::unordered_map<RpcId, std::string>& Rpc::names() { return registry().names; }
std::unordered_map<std::uint32_t, PendingRpc>& Rpc::pending() { return pending_map(); }

std::uint32_t Rpc::next_request_id() { return _next_id++; }

void Rpc::bind_raw(std::string_view name, RpcHandler handler) {
    if (name.empty())
        throw std::runtime_error("[Rpc] empty name");

    const RpcId id = rpc_id(name);

    auto& reg = registry();
    auto existing = reg.names.find(id);
    if (existing != reg.names.end() && existing->second != name) {
        std::printf("[Rpc] WARNING: id collision between '%s' and '%s' (id %u)\n",
                    existing->second.c_str(), std::string(name).c_str(), id);
    }

    reg.handlers[id] = std::move(handler);
    reg.names[id] = std::string(name);
}

void Rpc::bind(std::string_view name, RpcHandler handler) {
    bind_raw(name, std::move(handler));
}

bool Rpc::has(std::string_view name) {
    return handlers().find(rpc_id(name)) != handlers().end();
}

RpcId Rpc::id(std::string_view name) {
    return rpc_id(name);
}

std::string_view Rpc::name_of(RpcId id) {
    auto& n = names();
    auto it = n.find(id);
    if (it == n.end()) return {};
    return it->second;
}

void Rpc::unbind(std::string_view name) {
    const RpcId id = rpc_id(name);
    handlers().erase(id);
    names().erase(id);
}

void Rpc::clear_handlers() {
    handlers().clear();
    names().clear();
}

std::uint32_t Rpc::invoke_async(
    std::string_view name,
    RpcReplyCallback on_reply,
    RpcTimeoutCallback on_timeout,
    std::vector<std::uint8_t> args,
    float timeout_sec
) {
    return invoke_async(
        kServerPeerId, name, std::move(on_reply), std::move(on_timeout),
        std::move(args), timeout_sec);
}

std::uint32_t Rpc::invoke_async(
    PeerId target,
    std::string_view name,
    RpcReplyCallback on_reply,
    RpcTimeoutCallback on_timeout,
    std::vector<std::uint8_t> args,
    float timeout_sec
) {
    const RpcId rpc = rpc_id(name);
    const std::uint32_t rid = next_request_id();

    pending()[rid] = PendingRpc{
        std::move(on_reply),
        std::move(on_timeout),
        timeout_sec
    };

    Network::send(target, MsgType::RpcCall, SendFlag::Reliable,
        [rid, rpc, args = std::move(args)](BinaryWriter& w) mutable {
            write_call(w, rpc, rid);
            if (!args.empty())
                w.write(args.data(), args.size());
        });

    Network::stats().rpcs_sent++;
    return rid;
}

void Rpc::dispatch_local(RpcId id, BinaryReader& r) {
    auto& map = handlers();
    auto it = map.find(id);
    if (it == map.end()) return;

    RpcContext ctx;
    ctx.sender = kServerPeerId;
    ctx.request_id = 0;
    try {
        it->second(ctx, r);
    } catch (const std::exception& e) {
        std::printf("[Rpc] local call failed: %s\n", e.what());
    } catch (...) {
        std::printf("[Rpc] local call failed with an unknown exception\n");
    }
}

void Rpc::handle_call(PeerId sender, bool on_server, BinaryReader& r) {
    RpcId id = 0;
    if (!r.read(id)) return;

    std::uint32_t request_id = 0;
    if (!r.read(request_id)) return;

    auto& map = handlers();
    auto it = map.find(id);
    if (it == map.end()) {
        if ((++g_unknown_calls % 1024) == 1)
            std::printf("[Rpc] unknown rpc id %u from peer %u\n", id, sender);
        return;
    }

    RpcContext ctx;
    ctx.sender = on_server ? sender : kServerPeerId;
    ctx.from_server = !on_server;
    ctx.request_id = request_id;

    it->second(ctx, r);

    if (!ctx.has_reply) return;

    PeerId back = ctx.reply_target != kInvalidPeerId
        ? ctx.reply_target
        : (on_server ? sender : kServerPeerId);
    if (back == kInvalidPeerId) return;

    Network::send(back, MsgType::RpcReply, SendFlag::Reliable,
        [&ctx, request_id](BinaryWriter& w) {
            w.write(request_id);
            if (!ctx.reply_payload.empty())
                w.write(ctx.reply_payload.data(), ctx.reply_payload.size());
        });
}

void Rpc::handle_reply(BinaryReader& r) {
    std::uint32_t rid = 0;
    if (!r.read(rid)) return;

    auto& p = pending();
    auto it = p.find(rid);
    if (it == p.end()) return;

    PendingRpc entry = std::move(it->second);
    p.erase(it);

    if (entry.on_reply) entry.on_reply(r);
}

void Rpc::update(float dt) {
    auto& p = pending();
    for (auto it = p.begin(); it != p.end(); ) {
        it->second.deadline -= dt;
        if (it->second.deadline <= 0.f) {
            if (it->second.on_timeout) it->second.on_timeout();
            it = p.erase(it);
        } else {
            ++it;
        }
    }
}

void Rpc::clear() {
    pending().clear();
    _next_id = 1;
}

}
