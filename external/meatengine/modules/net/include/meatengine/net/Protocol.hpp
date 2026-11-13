#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include <meatengine/net/Serialization.hpp>
#include <meatengine/net/Types.hpp>

namespace me::net {

constexpr std::uint32_t kProtocolVersion = 2;

struct PeerInfo {
    ConnectionID conn = k_HSteamNetConnection_Invalid;
    std::string address;
    float latency_ms = 0.f;
};

enum class SendFlag : std::uint8_t {
    Unreliable = 0,
    UnreliableNoNagle,
    Reliable,
    ReliableNoNagle,
    UnreliableNoDelay,
};

inline int gns_flag(SendFlag f) {
    switch (f) {
        case SendFlag::Unreliable: return k_nSteamNetworkingSend_Unreliable;
        case SendFlag::UnreliableNoNagle: return k_nSteamNetworkingSend_UnreliableNoNagle;
        case SendFlag::Reliable: return k_nSteamNetworkingSend_Reliable;
        case SendFlag::ReliableNoNagle: return k_nSteamNetworkingSend_ReliableNoNagle;
        case SendFlag::UnreliableNoDelay: return k_nSteamNetworkingSend_UnreliableNoDelay;
    }
    return k_nSteamNetworkingSend_Reliable;
}

inline bool is_reliable(SendFlag f) {
    return f == SendFlag::Reliable || f == SendFlag::ReliableNoNagle;
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

    Batch,

    Count
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
        case MsgType::Batch: return "Batch";
        case MsgType::Count: return "Count";
    }
    return "unknown";
}


using RpcId = std::uint32_t;

constexpr RpcId kInvalidRpcId = 0;

inline RpcId rpc_id(std::string_view name) noexcept {
    std::uint32_t h = 2166136261u;
    for (char c : name) {
        h ^= static_cast<std::uint8_t>(c);
        h *= 16777619u;
    }
    return h == 0 ? 1u : h;
}


constexpr std::size_t kDefaultMaxBatchBytes = 48 * 1024;

struct Envelope {
    MsgType type = MsgType::Handshake;
    SendFlag send_flag = SendFlag::Reliable;
    std::uint32_t seq = 0;
};

constexpr std::size_t kEnvelopeHeaderSize = 6;

void patch_envelope(
    std::uint8_t* dst,
    MsgType type,
    SendFlag flag,
    std::uint32_t seq
);

void write_envelope(BinaryWriter& w, const Envelope& env);

bool read_envelope(BinaryReader& r, Envelope& out);

}
