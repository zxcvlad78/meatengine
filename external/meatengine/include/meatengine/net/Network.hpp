#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <entt/entity/entity.hpp>

#include <meatengine/net/Client.hpp>
#include <meatengine/net/Server.hpp>
#include <meatengine/net/Protocol.hpp>

namespace me::net {

class Network {
public:
    Network() = delete;

    using PeerEventCallback = std::function<void(PeerId)>;

    static Server* server();
    static Client* client();

    static PeerId peer_id();
    static void set_peer_id(PeerId id);

    static bool start_server(uint16_t port = kDefaultPort);
    static bool connect(const std::string& address);
    static void disconnect();

    static void assign(uint64_t id, entt::entity e);
    static entt::entity resolve(uint64_t id);
    static uint64_t resolve(entt::entity e);
    static void remove(uint64_t id);
    static void remove(entt::entity e);

    static bool is_server();
    static bool is_client();
    static bool is_authority(entt::entity e);

    static uint32_t next_net_id();
    static PeerId next_peer_id();

    static const std::unordered_map<PeerId, PeerInfo>& peers();
    static bool peer(PeerId id, PeerInfo& out);
    static void remove_peer(PeerId id);
    static void set_peer_address(PeerId id, const std::string& address);
    static void set_peer_latency(PeerId id, float ms);

    static void send(PeerId target, MsgType type, SendFlag flag, const std::vector<uint8_t>& payload);
    static void send(MsgType type, SendFlag flag, const std::vector<uint8_t>& payload, PeerId except = kInvalidPeerId);

    static void handle_message(ConnectionID from_conn, const std::uint8_t* data, std::uint32_t size);

    static std::uint32_t now_ms();
    static entt::entity spawn_entity(const std::string& packed_id, PeerId owner = kServerPeerId);
    static void despawn_entity(entt::entity e);
    static void set_authority(entt::entity e, PeerId owner);

    static void set_on_peer_connected(PeerEventCallback cb);
    static void set_on_peer_disconnected(PeerEventCallback cb);

    static void set_auto_cleanup_owned(bool v);
    static bool get_auto_cleanup_owned();

private:
    inline static std::unordered_map<uint64_t, entt::entity> _nid_map;
    inline static std::unordered_map<entt::entity, uint64_t> _entity_map;
    inline static std::unordered_map<PeerId, PeerInfo> _peers;

    inline static Server* _server = nullptr;
    inline static Client* _client = nullptr;

    inline static PeerId _peer_id = kServerPeerId;

    inline static uint32_t _next_net_id  = 1;
    inline static PeerId _next_peer_id = 1;
    inline static uint32_t _seq_counter = 1;
};

}