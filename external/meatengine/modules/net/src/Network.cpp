#include <meatengine/net/Network.hpp>
#include <meatengine/net/Rpc.hpp>

#include <meatengine/app/MainLoop.hpp>

#include <meatengine/net/cmp/Authority.hpp>

#include <cstdio>

#include "NetworkInternal.hpp"

namespace me::net::detail {

void on_client_connected(ConnectionID conn, const char* addr) {
    auto* server = Network::server();
    if (!server) return;

    const PeerId pid = Network::add_peer(conn, addr ? addr : "");
    std::printf("[Network][Server] conn=%u peer %u connected (%s)\n", conn, pid, addr ? addr : "?");
}

void on_client_disconnected(ConnectionID conn, int reason, const char* dbg) {
    auto* server = Network::server();
    const PeerId pid = server ? server->PeerOf(conn) : kInvalidPeerId;
    if (pid == kInvalidPeerId) return;

    std::printf("[Network][Server] conn=%u peer %u disconnected: %d (%s)\n",
                conn, pid, reason, dbg ? dbg : "");

    {
        BinaryWriter w;
        w.write(pid);
        Network::send(
            MsgType::PeerDisconnected,
            SendFlag::Reliable,
            w.buffer(), pid
        );
    }

    safe_callback(on_peer_disconnected_cb(), pid, "peer_disconnected");

    if (auto_cleanup_owned_flag()) {
        auto& reg = MainLoop::get_registry();
        std::vector<entt::entity> to_destroy;
        for (auto [e, a] : reg.view<me::net::cmp::Authority>().each())
            if (a.id == pid && reg.valid(e)) to_destroy.push_back(e);
        for (auto e : to_destroy)
            Network::despawn_entity(e);
    }

    if (server) server->UnassignPeer(conn);
    Network::remove_peer(pid);
}

void on_server_message(PeerId peer, ConnectionID conn, const void* data, std::uint32_t size) {
    (void)conn;
    Network::handle_message(peer, static_cast<const std::uint8_t*>(data), size);
}

void on_connected() {
    BinaryWriter w;
    w.write(kProtocolVersion);
    Network::send(
        kServerPeerId,
        MsgType::Handshake,
        SendFlag::Reliable,
        w.buffer()
    );
}

void on_disconnected(int reason, const char* dbg) {
    std::printf("[Network][Client] disconnected: %d (%s)\n", reason, dbg ? dbg : "");
}

void on_server_message_client(const void* data, std::uint32_t size) {
    Network::handle_message(
        kServerPeerId,
        static_cast<const std::uint8_t*>(data),
        size
    );
}

}

namespace me::net {

Server* Network::server() { return _server; }
Client* Network::client() { return _client; }

bool Network::is_server() { return _server != nullptr; }
bool Network::is_client() { return _client != nullptr; }

PeerId Network::peer_id() { return _peer_id; }
void Network::set_peer_id(PeerId id) { _peer_id = id; }

bool Network::start_server(std::uint16_t port) {
    if (_server) return false;

    delete _client;
    _client = nullptr;

    auto* server = new Server();
    if (!server->Start(port)) {
        delete server;
        return false;
    }

    server->SetOnClientConnected(detail::on_client_connected);
    server->SetOnClientDisconnected(detail::on_client_disconnected);
    server->SetOnMessageReceived(detail::on_server_message);

    _server = server;
    _peer_id = kServerPeerId;
    _peers.clear();

    std::printf("[Network] started server on :%u\n", port);
    return true;
}

bool Network::connect(const std::string& address) {
    disconnect();

    auto* c = new Client();
    if (!c->Connect(address)) {
        delete c;
        return false;
    }

    c->SetOnConnected(detail::on_connected);
    c->SetOnDisconnected(detail::on_disconnected);
    c->SetOnMessageReceived(detail::on_server_message_client);

    _client = c;
    _peers[kServerPeerId] = PeerInfo{};
    std::printf("[Network] connecting to %s\n", address.c_str());
    return true;
}

void Network::disconnect() {
    delete _server; _server = nullptr;
    delete _client; _client = nullptr;

    _nid_map.clear();
    _entity_map.clear();
    _peers.clear();
    _batches.clear();
    _broadcast_packet.clear();
    detail::packed_ids().clear();
    Rpc::clear();

    _peer_id = kServerPeerId;
    _next_net_id = 1;
    _next_peer_id = 1;
    _seq_counter = 1;
    reset_stats();
}

void Network::set_batching(bool enabled) {
    if (_batching == enabled) return;
    _batching = enabled;
    if (!enabled) flush();
}

bool Network::batching() { return _batching; }

void Network::set_max_batch_bytes(std::size_t bytes) {
    _max_batch_bytes = bytes < 1024 ? 1024 : bytes;
}

std::size_t Network::max_batch_bytes() { return _max_batch_bytes; }

}
