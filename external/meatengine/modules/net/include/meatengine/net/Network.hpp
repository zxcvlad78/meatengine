#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <entt/entity/entity.hpp>

#include <meatengine/net/Client.hpp>
#include <meatengine/net/Packet.hpp>
#include <meatengine/net/Protocol.hpp>
#include <meatengine/net/Server.hpp>

namespace me::net {

struct NetworkStats {
    std::uint64_t messages_sent = 0;
    std::uint64_t messages_received = 0;
    std::uint64_t bytes_sent = 0;
    std::uint64_t bytes_received = 0;
    std::uint64_t batches_sent = 0;
    std::uint64_t rpcs_sent = 0;
    std::uint64_t rpcs_received = 0;
    std::uint64_t dropped_sends = 0;

    double messages_per_sec = 0.0;
    double rpcs_per_sec = 0.0;
    double megabytes_per_sec = 0.0;

    void reset() { *this = NetworkStats{}; }
};

class Network {
public:
    Network() = delete;

    using PeerEventCallback = std::function<void(PeerId)>;

    static Server* server();
    static Client* client();

    static PeerId peer_id();
    static void set_peer_id(PeerId id);

    static bool start_server(std::uint16_t port = kDefaultPort);
    static bool connect(const std::string& address);
    static void disconnect();

    static bool is_server();
    static bool is_client();
    static bool is_authority(entt::entity e);

    static void assign(std::uint64_t net_id, entt::entity e);
    static entt::entity resolve(std::uint64_t net_id);
    static std::uint64_t resolve(entt::entity e);
    static void remove(std::uint64_t net_id);
    static void remove(entt::entity e);

    static entt::entity spawn_entity(const std::string& packed_id, PeerId owner = kServerPeerId);
    static void despawn_entity(entt::entity e);
    static void set_authority(entt::entity e, PeerId owner);

    static std::uint32_t next_net_id();
    static PeerId next_peer_id();

    static const std::unordered_map<PeerId, PeerInfo>& peers();
    static bool peer(PeerId id, PeerInfo& out);
    static void remove_peer(PeerId id);
    static void set_peer_address(PeerId id, const std::string& address);
    static void set_peer_latency(PeerId id, float ms);
    static std::size_t peer_count();

    static PeerId add_peer(ConnectionID conn, const std::string& address);
    static void register_peer(PeerId id, ConnectionID conn, const std::string& address);


    static void send_record(PeerId target, MsgType type, SendFlag flag, const void* data, std::size_t size);

    template<typename F>
    static void send_record(PeerId target, MsgType type, SendFlag flag, F&& write) {
        if (!_batching) {
            Packet packet;
            packet.begin(type, flag);
            write(packet.writer());
            send_packet(target, packet);
            return;
        }

        Packet& batch = batch_for(target);
        if (batch.type() != MsgType::Batch || batch.flag() != flag) {
            flush(target);
            batch.begin(MsgType::Batch, flag);
        } else if (batch.body_size() >= _max_batch_bytes) {
            flush(target);
            batch.begin(MsgType::Batch, flag);
        }
        batch.record(type, flag, std::forward<F>(write));
    }

    static void send(PeerId target, MsgType type, SendFlag flag, const void* data, std::size_t size);
    static void send(PeerId target, MsgType type, SendFlag flag, const std::vector<std::uint8_t>& payload);

    template<typename F>
    static void send(PeerId target, MsgType type, SendFlag flag, F&& write) {
        if (_batching) {
            send_record(target, type, flag, std::forward<F>(write));
            return;
        }
        Packet packet;
        packet.begin(type, flag);
        write(packet.writer());
        send_packet(target, packet);
    }

    static void send(MsgType type, SendFlag flag, const void* data, std::size_t size, PeerId except = kInvalidPeerId);
    static void send(MsgType type, SendFlag flag, const std::vector<std::uint8_t>& payload, PeerId except = kInvalidPeerId);

    template<typename F>
    static void send(MsgType type, SendFlag flag, F&& write, PeerId except = kInvalidPeerId) {
        if (_batching) {
            if (except != kInvalidPeerId) {
                Packet packet;
                packet.begin(type, flag);
                write(packet.writer());
                send_packet_except(packet, except);
                return;
            }
            Packet& batch = broadcast_batch();
            if (batch.type() != MsgType::Batch || batch.flag() != flag) {
                flush_broadcast();
                batch.begin(MsgType::Batch, flag);
            } else if (batch.body_size() >= _max_batch_bytes) {
                flush_broadcast();
                batch.begin(MsgType::Batch, flag);
            }
            batch.record(type, flag, std::forward<F>(write));
            return;
        }
        Packet packet;
        packet.begin(type, flag);
        write(packet.writer());
        send_packet(packet, except);
    }

    static void send_packet(PeerId target, Packet& packet);
    static void send_packet(Packet& packet, PeerId except = kInvalidPeerId);
    static void send_packet_except(Packet& packet, PeerId except);


    static void set_batching(bool enabled);
    static bool batching();

    static void set_max_batch_bytes(std::size_t bytes);
    static std::size_t max_batch_bytes();

    static void flush();
    static void flush(PeerId target);

    static void handle_message(PeerId sender, const std::uint8_t* data, std::uint32_t size);
    static std::uint32_t now_ms();

    static void set_on_peer_connected(PeerEventCallback cb);
    static void set_on_peer_disconnected(PeerEventCallback cb);

    static void set_auto_cleanup_owned(bool v);
    static bool get_auto_cleanup_owned();

    static NetworkStats& stats();
    static void reset_stats();
    static void update_stats();

private:
    static void deliver(PeerId target, const void* data, std::size_t size, SendFlag flag);
    static void flush_broadcast();

    static Packet& batch_for(PeerId target);
    static Packet& broadcast_batch();

    static ConnectionID conn_of(PeerId id);

    inline static std::unordered_map<std::uint64_t, entt::entity> _nid_map;
    inline static std::unordered_map<entt::entity, uint64_t> _entity_map;
    inline static std::unordered_map<PeerId, PeerInfo> _peers;

    inline static std::unordered_map<PeerId, Packet> _batches;

    inline static Server* _server = nullptr;
    inline static Client* _client = nullptr;

    inline static PeerId _peer_id = kServerPeerId;

    inline static uint32_t _next_net_id = 1;
    inline static PeerId _next_peer_id = 1;
    inline static uint32_t _seq_counter = 1;

    inline static bool _batching = false;
    inline static std::size_t _max_batch_bytes = kDefaultMaxBatchBytes;
    inline static Packet _broadcast_packet;

    inline static NetworkStats _stats;

    inline static std::uint32_t _stats_window_ms = 0;
    inline static std::uint64_t _stats_win_messages = 0;
    inline static std::uint64_t _stats_win_bytes = 0;
    inline static std::uint64_t _stats_win_rpcs = 0;
};

}
