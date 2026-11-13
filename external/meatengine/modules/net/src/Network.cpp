#include <meatengine/net/Network.hpp>
#include <meatengine/net/Replication.hpp>
#include <meatengine/net/Rpc.hpp>

#include <meatengine/app/MainLoop.hpp>
#include <meatengine/ecs/PackedEntity.hpp>

#include <meatengine/net/cmp/Authority.hpp>
#include <meatengine/net/cmp/NetId.hpp>

#include <chrono>
#include <cstdio>

namespace me::net {

namespace {

std::unordered_map<std::uint32_t, std::string>& packed_ids() {
    static std::unordered_map<std::uint32_t, std::string> m;
    return m;
}

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

void on_server_message(PeerId peer, ConnectionID conn, const void* data, uint32_t size) {
    (void)conn;
    Network::handle_message(peer, static_cast<const uint8_t*>(data), size);
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

void on_server_message_client(const void* data, uint32_t size) {
    Network::handle_message(
        kServerPeerId,
        static_cast<const uint8_t*>(data),
        size
    );
}


void dispatch_message(MsgType type, PeerId sender, BinaryReader& r);

void dispatch_batch(PeerId sender, BinaryReader& r) {
    std::uint32_t count = 0;
    if (!read_varint(r, count)) return;

    for (std::uint32_t i = 0; i < count; ++i) {
        std::uint8_t t = 0;
        std::uint32_t size = 0;
        if (!r.read(t)) return;
        if (!read_varint(r, size)) return;
        if (size > r.remaining()) return;

        const std::uint8_t* payload = r.data() + r.position();
        BinaryReader sub(payload, size);
        dispatch_message(static_cast<MsgType>(t), sender, sub);
        if (!r.skip(size)) return;
    }
}

void dispatch_message(MsgType type, PeerId sender, BinaryReader& r) {
    const bool on_server = Network::is_server();

    switch (type) {
        case MsgType::Handshake: {
            if (on_server) {
                std::uint32_t proto = 0;
                if (!r.read(proto)) return;
                if (proto != kProtocolVersion) {
                    std::printf("[Network][Server] protocol mismatch conn=%u\n", sender);
                    if (auto* s = Network::server()) {
                        PeerInfo info;
                        if (Network::peer(sender, info))
                            s->Kick(info.conn, "protocol mismatch");
                    }
                    return;
                }

                const PeerId pid = sender;
                if (pid == kInvalidPeerId) return;

                Network::send(pid, MsgType::Handshake, SendFlag::Reliable,
                    [pid](BinaryWriter& w) {
                        w.write(pid);
                        w.write(kProtocolVersion);
                    });

                Network::send(MsgType::PeerConnected, SendFlag::Reliable,
                    [pid](BinaryWriter& w) { w.write(pid); }, pid);

                for (auto& [id, info] : Network::peers()) {
                    if (id == pid) continue;
                    Network::send(pid, MsgType::PeerConnected, SendFlag::Reliable,
                        [id](BinaryWriter& w) { w.write(id); });
                }

                auto& reg = MainLoop::get_registry();
                for (auto& [nid, packed_id_str] : packed_ids()) {
                    auto e = Network::resolve(nid);
                    if (e == entt::null) continue;
                    auto* a = reg.try_get<me::net::cmp::Authority>(e);
                    PeerId owner = a ? a->id : kServerPeerId;
                    Network::send(pid, MsgType::EntitySpawn, SendFlag::Reliable,
                        [nid, owner, &packed_id_str](BinaryWriter& w) {
                            w.write(nid);
                            w.write(owner);
                            w.write(packed_id_str);
                        });
                }

                safe_callback(on_peer_connected_cb(), pid, "peer_connected");
                std::printf("[Network][Server] peer %u joined\n", pid);
            }
            else {
                std::uint32_t pid = 0, proto = 0;
                if (!r.read(pid)) return;
                if (!r.read(proto)) return;
                if (proto != kProtocolVersion) {
                    std::printf("[Network][Client] protocol mismatch\n");
                    Network::disconnect();
                    return;
                }
                Network::set_peer_id(pid);
                std::printf("[Network][Client] assigned peer id %u\n", pid);
            }
            break;
        }

        case MsgType::PeerConnected: {
            if (on_server) return;
            std::uint32_t pid = 0;
            if (!r.read(pid)) return;
            Network::register_peer(pid, k_HSteamNetConnection_Invalid, "");
            std::printf("[Network][Client] peer %u connected\n", pid);
            break;
        }

        case MsgType::PeerDisconnected: {
            if (on_server) return;
            std::uint32_t pid = 0;
            if (!r.read(pid)) return;
            Network::remove_peer(pid);
            std::printf("[Network][Client] peer %u left\n", pid);
            break;
        }

        case MsgType::EntitySpawn: {
            if (on_server) return;
            std::uint32_t nid = 0, owner = 0;
            std::string packed_id;
            if (!r.read(nid)) return;
            if (!r.read(owner)) return;
            if (!r.read(packed_id)) return;

            auto& reg = MainLoop::get_registry();

            if (Network::resolve(nid) != entt::null) {
                auto e = Network::resolve(nid);
                if (reg.valid(e))
                    reg.emplace_or_replace<me::net::cmp::Authority>(e, me::net::cmp::Authority{owner});
                return;
            }

            auto e = PackedEntity::spawn(packed_id, reg);
            if (e == entt::null) {
                std::printf("[Network][Client] EntitySpawn: unknown '%s'\n", packed_id.c_str());
                return;
            }

            reg.emplace_or_replace<me::net::cmp::NetId>(e).value = nid;
            reg.emplace_or_replace<me::net::cmp::Authority>(e).id = owner;
            Network::assign(nid, e);
            break;
        }

        case MsgType::EntityDespawn: {
            if (on_server) return;
            std::uint32_t nid = 0;
            if (!r.read(nid)) return;
            auto e = Network::resolve(nid);
            if (e == entt::null) return;
            auto& reg = MainLoop::get_registry();
            if (reg.valid(e)) reg.destroy(e);
            Network::remove(nid);
            break;
        }

        case MsgType::EntitySnapshot: {
            std::uint32_t count = 0;
            if (!r.read(count)) return;

            auto& reg = MainLoop::get_registry();
            for (std::uint32_t i = 0; i < count; ++i) {
                std::uint32_t nid = 0;
                std::uint64_t mask = 0;
                if (!r.read(nid)) return;
                if (!r.read(mask)) return;

                auto e = Network::resolve(nid);

                if (e == entt::null || !reg.valid(e)) {
                    if (!Replication::skip_components(r, mask)) return;
                    continue;
                }

                if (on_server) {
                    auto* a = reg.try_get<me::net::cmp::Authority>(e);
                    if (!a || a->id != sender) {
                        if (!Replication::skip_components(r, mask)) return;
                        continue;
                    }
                }

                if (!on_server && Network::is_authority(e)) {
                    if (!Replication::skip_components(r, mask)) return;
                    continue;
                }

                if (!Replication::read_components(r, reg, e, mask)) return;
            }
            break;
        }

        case MsgType::EntityAuthority: {
            std::uint32_t nid = 0, new_owner = 0;
            if (!r.read(nid)) return;
            if (!r.read(new_owner)) return;

            auto e = Network::resolve(nid);
            if (e == entt::null) return;

            auto& reg = MainLoop::get_registry();
            if (reg.valid(e))
                reg.emplace_or_replace<me::net::cmp::Authority>(e).id = new_owner;
            break;
        }

        case MsgType::RpcCall: {
            Rpc::handle_call(sender, on_server, r);
            Network::stats().rpcs_received++;
            break;
        }

        case MsgType::RpcReply: {
            Rpc::handle_reply(r);
            break;
        }

        case MsgType::Ping: {
            std::uint32_t stamp = 0;
            if (!r.read(stamp)) return;

            auto write_pong = [stamp](BinaryWriter& w) { w.write(stamp); };

            if (on_server) {
                if (sender != kInvalidPeerId)
                    Network::send(sender, MsgType::Pong, SendFlag::UnreliableNoNagle, write_pong);
            } else {
                Network::send(kServerPeerId, MsgType::Pong, SendFlag::UnreliableNoNagle, write_pong);
            }
            break;
        }

        case MsgType::Pong: {
            std::uint32_t stamp = 0;
            if (!r.read(stamp)) return;
            std::uint32_t rtt = Network::now_ms() - stamp;
            if (on_server) {
                if (sender != kInvalidPeerId)
                    Network::set_peer_latency(sender, static_cast<float>(rtt));
            } else {
                Network::set_peer_latency(kServerPeerId, static_cast<float>(rtt));
            }
            break;
        }

        case MsgType::Batch: {
            dispatch_batch(sender, r);
            break;
        }

        default: break;
    }
}

}


Server* Network::server() { return _server; }
Client* Network::client() { return _client; }

bool Network::is_server() { return _server != nullptr; }
bool Network::is_client() { return _client != nullptr; }

PeerId Network::peer_id() { return _peer_id; }
void Network::set_peer_id(PeerId id) { _peer_id = id; }

void Network::set_on_peer_connected(PeerEventCallback cb) {
    on_peer_connected_cb() = std::move(cb);
}
void Network::set_on_peer_disconnected(PeerEventCallback cb) {
    on_peer_disconnected_cb() = std::move(cb);
}
void Network::set_auto_cleanup_owned(bool v) { auto_cleanup_owned_flag() = v; }
bool Network::get_auto_cleanup_owned() { return auto_cleanup_owned_flag(); }

std::uint32_t Network::now_ms() {
    static const auto t0 = std::chrono::steady_clock::now();
    return static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count());
}

bool Network::start_server(std::uint16_t port) {
    if (_server) return false;

    delete _client;
    _client = nullptr;

    auto* server = new Server();
    if (!server->Start(port)) {
        delete server;
        return false;
    }

    server->SetOnClientConnected(on_client_connected);
    server->SetOnClientDisconnected(on_client_disconnected);
    server->SetOnMessageReceived(on_server_message);

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

    c->SetOnConnected(on_connected);
    c->SetOnDisconnected(on_disconnected);
    c->SetOnMessageReceived(on_server_message_client);

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
    packed_ids().clear();
    Rpc::clear();

    _peer_id = kServerPeerId;
    _next_net_id = 1;
    _next_peer_id = 1;
    _seq_counter = 1;
    reset_stats();
}


void Network::assign(std::uint64_t net_id, entt::entity e) {
    _nid_map[net_id] = e;
    _entity_map[e] = net_id;
}

entt::entity Network::resolve(std::uint64_t net_id) {
    auto it = _nid_map.find(net_id);
    return it == _nid_map.end() ? entt::null : it->second;
}

std::uint64_t Network::resolve(entt::entity e) {
    auto it = _entity_map.find(e);
    return it == _entity_map.end() ? 0u : it->second;
}

void Network::remove(std::uint64_t net_id) {
    auto it = _nid_map.find(net_id);
    if (it == _nid_map.end()) return;
    _entity_map.erase(it->second);
    _nid_map.erase(it);
    packed_ids().erase(static_cast<std::uint32_t>(net_id));
}

void Network::remove(entt::entity e) {
    auto it = _entity_map.find(e);
    if (it == _entity_map.end()) return;
    packed_ids().erase(static_cast<std::uint32_t>(it->second));
    _nid_map.erase(it->second);
    _entity_map.erase(it);
}

bool Network::is_authority(entt::entity e) {
    auto& reg = MainLoop::get_registry();
    auto* a = reg.try_get<me::net::cmp::Authority>(e);
    if (!a) return true;
    return a->id == _peer_id;
}

entt::entity Network::spawn_entity(const std::string& packed_id, PeerId owner) {
    if (!_server) return entt::null;

    auto& reg = MainLoop::get_registry();
    entt::entity e = PackedEntity::spawn(packed_id, reg);
    if (e == entt::null) return entt::null;

    std::uint32_t nid = next_net_id();

    reg.emplace_or_replace<me::net::cmp::NetId>(e, me::net::cmp::NetId{nid});
    reg.emplace_or_replace<me::net::cmp::Authority>(e, me::net::cmp::Authority{owner});

    assign(nid, e);
    packed_ids()[nid] = packed_id;

    Network::send(MsgType::EntitySpawn, SendFlag::Reliable,
        [nid, owner, &packed_id](BinaryWriter& w) {
            w.write(nid);
            w.write(owner);
            w.write(packed_id);
        }
    );

    return e;
}

void Network::despawn_entity(entt::entity e) {
    if (!_server) return;

    auto& reg = MainLoop::get_registry();
    if (!reg.valid(e)) return;

    std::uint32_t nid = 0;
    if (auto* n = reg.try_get<me::net::cmp::NetId>(e))
        nid = n->value;

    if (nid != 0)
        Network::send(MsgType::EntityDespawn, SendFlag::Reliable,
            [nid](BinaryWriter& w) { w.write(nid); });

    reg.destroy(e);
    if (nid != 0) remove(nid);
}

void Network::set_authority(entt::entity e, PeerId owner) {
    if (!_server) return;
    auto& reg = MainLoop::get_registry();
    if (!reg.valid(e)) return;
    if (!reg.all_of<me::net::cmp::NetId>(e)) return;

    reg.emplace_or_replace<me::net::cmp::Authority>(e, me::net::cmp::Authority{owner});

    std::uint32_t nid = reg.get<me::net::cmp::NetId>(e).value;
    Network::send(MsgType::EntityAuthority, SendFlag::Reliable,
        [nid, owner](BinaryWriter& w) {
            w.write(nid);
            w.write(owner);
        });
}


std::uint32_t Network::next_net_id() { return _next_net_id++; }
PeerId Network::next_peer_id() { return _next_peer_id++; }

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


Packet& Network::batch_for(PeerId target) {
    auto it = _batches.find(target);
    if (it != _batches.end()) return it->second;
    return _batches.emplace(target, Packet{}).first->second;
}

Packet& Network::broadcast_batch() {
    return _broadcast_packet;
}

void Network::send_record(PeerId target, MsgType type, SendFlag flag, const void* data, std::size_t size) {
    if (!_batching) {
        Packet packet;
        packet.begin(type, flag);
        packet.writer().write(data, size);
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
    batch.record(type, flag, data, size);
}

void Network::send(PeerId target, MsgType type, SendFlag flag, const void* data, std::size_t size) {
    if (_batching) {
        send_record(target, type, flag, data, size);
        return;
    }

    Packet packet;
    packet.begin(type, flag);
    packet.writer().write(data, size);
    send_packet(target, packet);
}

void Network::send(PeerId target, MsgType type, SendFlag flag, const std::vector<std::uint8_t>& payload) {
    send(target, type, flag, payload.data(), payload.size());
}

void Network::send(MsgType type, SendFlag flag, const void* data, std::size_t size, PeerId except) {
    if (_batching) {
        if (except != kInvalidPeerId) {
            Packet packet;
            packet.begin(type, flag);
            packet.writer().write(data, size);
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
        batch.record(type, flag, data, size);
        return;
    }

    Packet packet;
    packet.begin(type, flag);
    packet.writer().write(data, size);
    send_packet(packet, except);
}

void Network::send(MsgType type, SendFlag flag, const std::vector<std::uint8_t>& payload, PeerId except) {
    send(type, flag, payload.data(), payload.size(), except);
}

void Network::send_packet(PeerId target, Packet& packet) {
    if (packet.size() <= kEnvelopeHeaderSize) return;

    if (_batching && packet.type() != MsgType::Batch) {
        send_record(target, packet.type(), packet.flag(), packet.body_data(), packet.body_size());
        return;
    }

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);
    deliver(target, packet.data(), packet.size(), packet.flag());
}

void Network::send_packet(Packet& packet, PeerId except) {
    if (packet.size() <= kEnvelopeHeaderSize) return;

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);

    if (_server) {
        const ConnectionID ex = (except == kInvalidPeerId)
            ? k_HSteamNetConnection_Invalid
            : conn_of(except);
        _server->Broadcast(packet.data(), static_cast<std::uint32_t>(packet.size()), ex, packet.flag());
        _stats.messages_sent++;
        _stats.bytes_sent += packet.size();
    } else if (_client) {
        _client->Send(packet.data(), static_cast<std::uint32_t>(packet.size()), packet.flag());
        _stats.messages_sent++;
        _stats.bytes_sent += packet.size();
    }
}

void Network::deliver(PeerId target, const void* data, std::size_t size, SendFlag flag) {
    if (_server) {
        const ConnectionID conn = conn_of(target);
        if (conn == k_HSteamNetConnection_Invalid) {
            _stats.dropped_sends++;
            return;
        }
        _server->SendToClient(conn, data, static_cast<std::uint32_t>(size), flag);
        _stats.messages_sent++;
        _stats.bytes_sent += size;
    } else if (_client) {
        if (target != kServerPeerId) {
            _stats.dropped_sends++;
            return;
        }
        _client->Send(data, static_cast<std::uint32_t>(size), flag);
        _stats.messages_sent++;
        _stats.bytes_sent += size;
    }
}


void Network::send_packet_except(Packet& packet, PeerId except) {
    if (packet.size() <= kEnvelopeHeaderSize) return;

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);

    if (_server) {
        for (auto& [id, info] : _peers) {
            if (id == except) continue;
            deliver(id, packet.data(), packet.size(), packet.flag());
        }
    } else if (_client) {
        if (except != kServerPeerId)
            deliver(kServerPeerId, packet.data(), packet.size(), packet.flag());
    }
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

void Network::flush(PeerId target) {
    auto it = _batches.find(target);
    if (it == _batches.end()) return;

    Packet& packet = it->second;
    if (packet.record_count() == 0) return;

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);
    deliver(target, packet.data(), packet.size(), packet.flag());
    _stats.batches_sent++;
    packet.clear();
}

void Network::flush_broadcast() {
    Packet& packet = _broadcast_packet;
    if (packet.record_count() == 0) return;

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);

    if (_server) {
        _server->Broadcast(packet.data(), static_cast<std::uint32_t>(packet.size()), k_HSteamNetConnection_Invalid, packet.flag());
        _stats.messages_sent++;
        _stats.bytes_sent += packet.size();
    } else if (_client) {
        _client->Send(packet.data(), static_cast<std::uint32_t>(packet.size()), packet.flag());
        _stats.messages_sent++;
        _stats.bytes_sent += packet.size();
    }

    _stats.batches_sent++;
    packet.clear();
}

void Network::flush() {
    for (auto& [peer, packet] : _batches) {
        if (packet.record_count() == 0) continue;

        patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);
        deliver(peer, packet.data(), packet.size(), packet.flag());
        _stats.batches_sent++;
        packet.clear();
    }
    flush_broadcast();
}


void Network::handle_message(PeerId sender, const std::uint8_t* data, std::uint32_t size) {
    _stats.messages_received++;
    _stats.bytes_received += size;

    BinaryReader r(data, size);

    Envelope env{};
    if (!read_envelope(r, env)) return;

    dispatch_message(env.type, sender, r);
}


NetworkStats& Network::stats() { return _stats; }

void Network::reset_stats() {
    _stats.reset();
    _stats_window_ms = now_ms();
    _stats_win_messages = 0;
    _stats_win_bytes = 0;
    _stats_win_rpcs = 0;
}

void Network::update_stats() {
    const std::uint32_t now = now_ms();
    if (_stats_window_ms == 0) {
        _stats_window_ms = now;
        return;
    }

    const std::uint32_t dt = now - _stats_window_ms;
    if (dt < 500) return;

    const double secs = static_cast<double>(dt) * 0.001;
    _stats.messages_per_sec = static_cast<double>(_stats.messages_sent - _stats_win_messages) / secs;
    _stats.rpcs_per_sec = static_cast<double>(_stats.rpcs_sent - _stats_win_rpcs) / secs;
    _stats.megabytes_per_sec =
        static_cast<double>(_stats.bytes_sent - _stats_win_bytes) / secs / (1024.0 * 1024.0);

    _stats_window_ms = now;
    _stats_win_messages = _stats.messages_sent;
    _stats_win_bytes = _stats.bytes_sent;
    _stats_win_rpcs = _stats.rpcs_sent;
}

}
