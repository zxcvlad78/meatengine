#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <meatengine/net/Types.hpp>
#include <meatengine/net/Serialization.hpp>

namespace me::net {

using PeerId = std::uint32_t;
constexpr PeerId kInvalidPeerId = 0xFFFFFFFF;
constexpr PeerId kServerPeerId = 0;
constexpr std::uint32_t kProtocolVersion = 1;

struct PeerInfo {
    ConnectionID conn = k_HSteamNetConnection_Invalid;
    std::string address;
    float latency_ms = 0.f;
};

enum class SendFlag : std::uint8_t {
    Unreliable = 0,
    UnreliableNoNagle,
    Reliable,
    ReliableNoNagle
};

inline int gns_flag(SendFlag f) {
    switch (f) {
        case SendFlag::Unreliable: return k_nSteamNetworkingSend_Unreliable;
        case SendFlag::UnreliableNoNagle: return k_nSteamNetworkingSend_UnreliableNoNagle;
        case SendFlag::Reliable: return k_nSteamNetworkingSend_Reliable;
        case SendFlag::ReliableNoNagle: return k_nSteamNetworkingSend_ReliableNoNagle;
    }
    return k_nSteamNetworkingSend_Reliable;
}

enum class MsgType : std::uint8_t {
    Handshake = 0,
    PeerConnected,
    PeerDisconnected,
    Ping,
    Pong,

    EntitySpawn,
    EntityDespawn,
    EntitySnapshot,
    EntityAuthority,

    RpcCall,
    RpcReply,
};

inline const char* to_string(MsgType t) {
    switch (t) {
        case MsgType::Handshake: return "Handshake";
        case MsgType::PeerConnected: return "PeerConnected";
        case MsgType::PeerDisconnected: return "PeerDisconnected";
        case MsgType::Ping: return "Ping";
        case MsgType::Pong: return "Pong";
        case MsgType::EntitySpawn: return "EntitySpawn";
        case MsgType::EntityDespawn: return "EntityDespawn";
        case MsgType::EntitySnapshot: return "EntitySnapshot";
        case MsgType::EntityAuthority: return "EntityAuthority";
        case MsgType::RpcCall: return "RpcCall";
        case MsgType::RpcReply: return "RpcReply";
    }
    return "unknown";
}

inline void write_envelope(
    BinaryWriter& w,
    MsgType type,
    SendFlag send_flag,
    std::uint32_t seq,
    const std::vector<std::uint8_t>& payload
) {
    w.write(static_cast<std::uint8_t>(type));
    w.write(static_cast<std::uint8_t>(send_flag));
    w.write(seq);
    w.write(static_cast<std::uint16_t>(payload.size()));
    if (!payload.empty())
        w.write(payload.data(), payload.size());
}
 
struct Envelope {
    MsgType type = MsgType::Handshake;
    SendFlag send_flag = SendFlag::Reliable;
    std::uint32_t seq = 0;
    std::uint16_t size = 0;
};

inline void write_envelope(
    BinaryWriter& w,
    const Envelope& env,
    const std::vector<std::uint8_t>& payload
) {
    w.write(static_cast<std::uint8_t>(env.type));
    w.write(static_cast<std::uint8_t>(env.send_flag));
    w.write(env.seq);
    w.write(static_cast<std::uint16_t>(payload.size()));
    if (!payload.empty()) w.write(payload.data(), payload.size());
}

inline bool read_envelope(BinaryReader& r, Envelope& out) {
    std::uint8_t t, c;
    if (!r.read(t)) return false;
    if (!r.read(c)) return false;
    if (!r.read(out.seq)) return false;
    if (!r.read(out.size)) return false;
    out.type = static_cast<MsgType>(t);
    out.send_flag = static_cast<SendFlag>(c);
    return true;
}

} // namespace me::net