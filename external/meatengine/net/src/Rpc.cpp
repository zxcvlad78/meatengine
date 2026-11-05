#include <meatengine/net/Rpc.hpp>
#include <meatengine/net/Network.hpp>

#include <cstdio>
#include <stdexcept>
#include <utility>

namespace me::net {

std::unordered_map<std::string, RpcHandler>& Rpc::handlers() {
    static std::unordered_map<std::string, RpcHandler> h;
    return h;
}

std::unordered_map<std::uint32_t, PendingRpc>& Rpc::pending() {
    static std::unordered_map<std::uint32_t, PendingRpc> p;
    return p;
}

std::uint32_t Rpc::next_request_id() { return _next_id++; }

void Rpc::bind(const std::string& name, RpcHandler h) {
    if (name.empty())
        throw std::runtime_error("[Rpc] empty name");
    handlers()[name] = std::move(h);
}

bool Rpc::has(const std::string& name) { return handlers().count(name) > 0; }

void Rpc::run_local_handler(const std::string& name, const std::vector<std::uint8_t>& args) {
    auto it = handlers().find(name);
    if (it == handlers().end()) return;

    BinaryReader rr(args.data(), args.size());
    RpcContext ctx{};
    ctx.sender = kServerPeerId;
    ctx.request_id = 0;
    try {
        it->second(ctx, rr);
    } catch (const std::exception& e) {
        std::printf("[Rpc] broadcast '%s': %s\n", name.c_str(), e.what());
    }
}

std::uint32_t Rpc::invoke_async(
    const std::string& name,
    std::function<void(BinaryReader&)> on_reply,
    std::function<void()> on_timeout,
    std::vector<std::uint8_t> args,
    float timeout_sec
) {
    std::uint32_t rid = next_request_id();
    pending()[rid] = PendingRpc{
        std::move(on_reply), std::move(on_timeout), timeout_sec
    };

    Network::send(kServerPeerId, MsgType::RpcCall, SendFlag::Reliable,
        [rid, name_copy = name,
         args_copy = std::move(args)](BinaryWriter& w) mutable {
            w.write(rid);
            w.write(name_copy);
            if (!args_copy.empty())
                w.write(args_copy.data(), args_copy.size());
        });
    return rid;
}

std::uint32_t Rpc::invoke_async(
    PeerId target,
    const std::string& name,
    std::function<void(BinaryReader&)> on_reply,
    std::function<void()> on_timeout,
    std::vector<std::uint8_t> args,
    float timeout_sec
) {
    std::uint32_t rid = next_request_id();
    pending()[rid] = PendingRpc{
        std::move(on_reply), std::move(on_timeout), timeout_sec
    };

    Network::send(target, MsgType::RpcCall, SendFlag::Reliable,
        [rid, name_copy = name,
         args_copy = std::move(args)](BinaryWriter& w) mutable {
            w.write(rid);
            w.write(name_copy);
            if (!args_copy.empty())
                w.write(args_copy.data(), args_copy.size());
        });
    return rid;
}

void Rpc::handle_call(PeerId sender, bool on_server, BinaryReader& r) {
    std::uint32_t rid = 0;
    std::string   name;
    if (!r.read(rid)) return;
    if (!r.read(name)) return;

    RpcContext ctx{};
    ctx.sender = on_server ? sender : kServerPeerId;
    ctx.from_server = !on_server;
    ctx.request_id  = rid;

    auto it = handlers().find(name);
    if (it == handlers().end()) {
        std::printf("[Rpc] unhandled '%s' from peer %u\n", name.c_str(), sender);
        return;
    }
    it->second(ctx, r);

    if (!ctx.has_reply) return;

    PeerId back = on_server ? sender : kServerPeerId;
    if (back == kInvalidPeerId) return;

    Network::send(back, MsgType::RpcReply, SendFlag::Reliable,
        [&ctx, rid](BinaryWriter& w) {
            w.write(rid);
            if (!ctx.reply_payload.empty())
                w.write(ctx.reply_payload.data(), ctx.reply_payload.size());
        });
}

void Rpc::handle_reply(BinaryReader& r) {
    std::uint32_t rid = 0;
    if (!r.read(rid)) return;

    auto it = pending().find(rid);
    if (it == pending().end()) return;

    PendingRpc p = std::move(it->second);
    pending().erase(it);

    if (p.on_reply) p.on_reply(r);
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