#include <meatengine/net/Network.hpp>

#include <cstdio>
#include <utility>

#include "NetworkInternal.hpp"

namespace me::net::detail {

Network::PeerEventCallback& on_peer_connected_cb() {
    static Network::PeerEventCallback cb;
    return cb;
}

Network::PeerEventCallback& on_peer_disconnected_cb() {
    static Network::PeerEventCallback cb;
    return cb;
}

bool& auto_cleanup_owned_flag() {
    static bool v = true;
    return v;
}

void safe_callback(const Network::PeerEventCallback& cb, PeerId pid, const char* tag) {
    if (!cb) return;
    try { cb(pid); }
    catch (const std::exception& e) {
        std::printf("[Network][%s] callback threw: %s\n", tag, e.what());
    }
    catch (...) {
        std::printf("[Network][%s] callback threw unknown exception\n", tag);
    }
}

}

namespace me::net {

void Network::set_on_peer_connected(PeerEventCallback cb) {
    detail::on_peer_connected_cb() = std::move(cb);
}

void Network::set_on_peer_disconnected(PeerEventCallback cb) {
    detail::on_peer_disconnected_cb() = std::move(cb);
}

void Network::set_auto_cleanup_owned(bool v) { detail::auto_cleanup_owned_flag() = v; }
bool Network::get_auto_cleanup_owned() { return detail::auto_cleanup_owned_flag(); }

const std::unordered_map<PeerId, PeerInfo>& Network::peers() { return _peers; }

bool Network::peer(PeerId id, PeerInfo& out) {
    auto it = _peers.find(id);
    if (it == _peers.end()) return false;
    out = it->second;
    return true;
}

void Network::remove_peer(PeerId id) {
    _peers.erase(id);
    _batches.erase(id);
}

void Network::set_peer_address(PeerId id, const std::string& addr) {
    auto it = _peers.find(id);
    if (it != _peers.end()) it->second.address = addr;
}

void Network::set_peer_latency(PeerId id, float ms) {
    auto it = _peers.find(id);
    if (it != _peers.end()) it->second.latency_ms = ms;
}

std::size_t Network::peer_count() { return _peers.size(); }

PeerId Network::add_peer(ConnectionID conn, const std::string& address) {
    const PeerId pid = next_peer_id();
    register_peer(pid, conn, address);
    return pid;
}

void Network::register_peer(PeerId id, ConnectionID conn, const std::string& address) {
    PeerInfo info;
    info.conn = conn;
    info.address = address;
    _peers[id] = std::move(info);
    if (_server && conn != k_HSteamNetConnection_Invalid)
        _server->AssignPeer(conn, id);
}

ConnectionID Network::conn_of(PeerId id) {
    auto it = _peers.find(id);
    return it == _peers.end()
        ? k_HSteamNetConnection_Invalid
        : it->second.conn;
}

}
