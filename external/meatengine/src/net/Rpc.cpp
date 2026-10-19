#include <meatengine/net/Rpc.hpp>
#include <meatengine/net/Network.hpp>

#include <cstdio>
#include <stdexcept>

namespace me::net {

// namespace rpc {
// void write_arg(BinaryWriter& w, std::int8_t v) { w.WriteInt8(v); }
// void write_arg(BinaryWriter& w, std::uint8_t v) { w.WriteUInt8(v); }
// void write_arg(BinaryWriter& w, std::int16_t v) { w.WriteInt16(v); }
// void write_arg(BinaryWriter& w, std::uint16_t v) { w.WriteUInt16(v); }
// void write_arg(BinaryWriter& w, std::int32_t v) { w.WriteInt32(v); }
// void write_arg(BinaryWriter& w, std::uint32_t v) { w.WriteUInt32(v); }
// void write_arg(BinaryWriter& w, std::int64_t v) { w.WriteInt64(v); }
// void write_arg(BinaryWriter& w, std::uint64_t v) { w.WriteUInt64(v); }
// void write_arg(BinaryWriter& w, float v) { w.WriteFloat(v); }
// void write_arg(BinaryWriter& w, double v) { w.WriteFloat(static_cast<float>(v)); }
// void write_arg(BinaryWriter& w, bool v) { w.WriteBool(v); }
// void write_arg(BinaryWriter& w, const std::string& v) { w.WriteString(v); }
// void write_arg(BinaryWriter& w, const char* v) { w.WriteString(v ? std::string{v} : std::string{}); }
// void write_arg(BinaryWriter& w, const std::vector<std::uint8_t>& v) {
//     if (!v.empty())
//         w.WriteBytes(v.data(), v.size());
// }
// }


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


void Rpc::send(
    PeerId target,
    std::uint32_t rid,
    const std::string& name,
    const std::vector<std::uint8_t>& args
) {
    BinaryWriter w;
    w.write(rid);
    w.write(name);
    if (!args.empty()) w.write(args.data(), args.size());

    Network::send(target, MsgType::RpcCall, SendFlag::Reliable, w.buffer());
}

void Rpc::send(
    std::uint32_t rid,
    const std::string& name,
    const std::vector<std::uint8_t>& args
) {
    BinaryWriter w;
    w.write(rid);
    w.write(name);
    if (!args.empty()) w.write(args.data(), args.size());

    if (Network::is_server()) {
        Network::send(
            MsgType::RpcCall,
            SendFlag::Reliable,
            w.buffer()
        );
    } else {
        Network::send(
            kServerPeerId,
            MsgType::RpcCall,
            SendFlag::Reliable,
            w.buffer()
        );
    }
}

void Rpc::broadcast(const std::string& name, const std::vector<std::uint8_t>& args) {
    if (Network::is_server()) {
        auto it = handlers().find(name);
        if (it == handlers().end()) {
            std::printf("[Rpc] broadcast: no handler '%s' on server\n", name.c_str());
            return;
        }
        try {
            BinaryReader reader(args);
            RpcContext ctx;
            ctx.sender = kServerPeerId;
            ctx.from_server = false;
            ctx.request_id = 0;
            it->second(ctx, reader);
        } catch (const std::exception& e) {
            std::printf("[Rpc] broadcast '%s' handler threw: %s\n", name.c_str(), e.what());
        }
    } else {
        BinaryWriter w;
        w.write(0);
        w.write(name);
        if (!args.empty()) w.write(args.data(), args.size());
        Network::send(kServerPeerId, MsgType::RpcCall, SendFlag::Reliable, w.buffer());
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
    send(kServerPeerId, rid, name, args);
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
    send(target, rid, name, args);
    return rid;
}

void Rpc::handle_call(PeerId sender, bool on_server, BinaryReader& r) {
    std::uint32_t rid = 0;
    std::string   name;
    if (!r.read(rid))  return;
    if (!r.read(name)) return;

    RpcContext ctx;
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

    BinaryWriter w;
    w.write(rid);
    if (!ctx.reply_payload.empty())
        w.write(ctx.reply_payload.data(), ctx.reply_payload.size());

    if (on_server) {
        if (sender != kInvalidPeerId)
            Network::send(
                sender,
                MsgType::RpcReply,
                SendFlag::Reliable,
                w.buffer()
            );
    } else {
        Network::send(
            kServerPeerId,
            MsgType::RpcReply,
            SendFlag::Reliable,
            w.buffer()
        );
    }
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