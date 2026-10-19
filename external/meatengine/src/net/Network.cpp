#include <meatengine/net/Network.hpp>
#include <meatengine/net/Replication.hpp>
#include <meatengine/net/Rpc.hpp>

#include <meatengine/MainLoop.hpp>
#include <meatengine/PackedEntity.hpp>

#include <meatengine/cmp/Camera.hpp>
#include <meatengine/cmp/net/NetId.hpp>
#include <meatengine/cmp/net/Authority.hpp>

#include <chrono>
#include <cstdio>

namespace me::net {

std::unordered_map<std::uint32_t, std::string>& packed_ids() {
    static std::unordered_map<std::uint32_t, std::string> m;
    return m;
}

static ConnectionID conn_of(PeerId id) {
    auto it = Network::peers().find(id);
    return it == Network::peers().end()
        ? k_HSteamNetConnection_Invalid
        : it->second.conn;
}

static PeerId peer_of_conn(ConnectionID c) {
    for (auto& [id, p] : Network::peers())
        if (p.conn == c) return id;
    return kInvalidPeerId;
}

std::uint32_t Network::now_ms() {
    static const auto t0 = std::chrono::steady_clock::now();
    return static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count());
}

void send_entity_spawn(PeerId target, std::uint32_t nid, PeerId owner, const std::string& packed_id) {
    BinaryWriter w;
    w.write(nid);
    w.write(owner);
    w.write(packed_id);
    Network::send(target, MsgType::EntitySpawn, SendFlag::Reliable, w.buffer());
}

void send_entity_spawn(std::uint32_t nid, PeerId owner, const std::string& packed_id, PeerId except) {
    BinaryWriter w;
    w.write(nid);
    w.write(owner);
    w.write(packed_id);
    Network::send(MsgType::EntitySpawn, SendFlag::Reliable, w.buffer(), except);
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

static void on_client_connected(ConnectionID conn, const char* addr) {
    printf("[Network][Server] conn=%u connected (%s)\n", conn, addr);
}

static void on_client_disconnected(ConnectionID conn, int reason, const char* dbg) {
    printf("[Network][Server] conn=%u disconnected: %d (%s)\n", conn, reason, dbg);

    PeerId pid = peer_of_conn(conn);
    if (pid == kInvalidPeerId) return;

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

    auto& reg = MainLoop::get_registry();

    if (auto_cleanup_owned_flag()) {
        auto& reg = MainLoop::get_registry();
        std::vector<entt::entity> to_destroy;
        for (auto [e, a] : reg.view<me::cmp::net::Authority>().each())
            if (a.id == pid && reg.valid(e)) to_destroy.push_back(e);
        for (auto e : to_destroy)
            Network::despawn_entity(e);
    }

    Network::remove_peer(pid);
}

static void on_server_message(ConnectionID conn, const void* data, uint32_t size) {
    Network::handle_message(conn, static_cast<const uint8_t*>(data), size);
}

static void on_connected() {
    printf("[Network][Client] connected, sending Handshake\n");
    BinaryWriter w;
    w.write(kProtocolVersion);
    Network::send(
        kServerPeerId,
        MsgType::Handshake,
        SendFlag::Reliable,
        w.buffer()
    );
}

static void on_disconnected(int reason, const char* dbg) {
    printf("[Network][Client] disconnected: %d (%s)\n", reason, dbg);
}

static void on_server_message_client(const void* data, uint32_t size) {
    Network::handle_message(
        k_HSteamNetConnection_Invalid,
        static_cast<const uint8_t*>(data),
        size
    );
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

static void patch_header(BinaryWriter& w, MsgType type, SendFlag flag, std::uint32_t seq) {
    const std::uint32_t plen = static_cast<std::uint32_t>(w.buffer().size() - 8);
    if (plen > 0xFFFFu)
        throw std::runtime_error("[Network] payload exceeds 64KB");
    w.patch_u8 (0, static_cast<std::uint8_t>(type));
    w.patch_u8 (1, static_cast<std::uint8_t>(flag));
    w.patch_u32(2, seq);
    w.patch_u16(6, static_cast<std::uint16_t>(plen));
}

void Network::finish_and_send(BinaryWriter& w, PeerId target, MsgType type, SendFlag flag) {
    patch_header(w, type, flag, _seq_counter++);
    const auto* d = w.buffer().data();
    const auto  n = static_cast<std::uint32_t>(w.buffer().size());

    if (_server) {
        auto conn = conn_of(target);
        if (conn == k_HSteamNetConnection_Invalid) return;
        _server->SendToClient(conn, d, n, flag == SendFlag::Reliable);
    } else if (_client && target == kServerPeerId) {
        _client->Send(d, n, flag == SendFlag::Reliable);
    }
}

void Network::finish_and_send(BinaryWriter& w, MsgType type, SendFlag flag, PeerId except) {
    patch_header(w, type, flag, _seq_counter++);
    const auto* d = w.buffer().data();
    const auto  n = static_cast<std::uint32_t>(w.buffer().size());

    if (_server) {
        auto ex = (except == kInvalidPeerId)
            ? k_HSteamNetConnection_Invalid
            : conn_of(except);
        _server->Broadcast(d, n, ex, flag == SendFlag::Reliable);
    } else if (_client) {
        _client->Send(d, n, flag == SendFlag::Reliable);
    }
}

entt::entity Network::spawn_entity(const std::string& packed_id, PeerId owner) {
    if (!_server) return entt::null;

    auto& reg = MainLoop::get_registry();
    entt::entity e = PackedEntity::spawn(packed_id, reg);
    if (e == entt::null) return entt::null;

    std::uint32_t nid = next_net_id();

    reg.emplace_or_replace<me::cmp::net::NetId>(e, me::cmp::net::NetId{nid});
    reg.emplace_or_replace<me::cmp::net::Authority>(e, me::cmp::net::Authority{owner});

    assign(nid, e);
    packed_ids()[nid] = packed_id;

    send(MsgType::EntitySpawn, SendFlag::Reliable,
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
    if (auto* n = reg.try_get<me::cmp::net::NetId>(e))
        nid = n->value;

    if (nid != 0)
        send(MsgType::EntityDespawn, SendFlag::Reliable, [nid](BinaryWriter& w) { w.write(nid); });

    reg.destroy(e);
    if (nid != 0) remove(nid);
}

void Network::set_authority(entt::entity e, PeerId owner) {
    if (!_server) return;
    auto& reg = MainLoop::get_registry();
    if (!reg.valid(e)) return;
    if (!reg.all_of<me::cmp::net::NetId>(e)) return;

    reg.emplace_or_replace<me::cmp::net::Authority>(e, me::cmp::net::Authority{owner});

    std::uint32_t nid = reg.get<me::cmp::net::NetId>(e).value;
    send(MsgType::EntityAuthority, SendFlag::Reliable,
        [nid, owner](BinaryWriter& w) {
            w.write(nid);
            w.write(owner);
        });
}

void Network::set_peer_address(PeerId id, const std::string& addr) {
    auto it = _peers.find(id);
    if (it != _peers.end()) it->second.address = addr;
}

void Network::set_peer_latency(PeerId id, float ms) {
    auto it = _peers.find(id);
    if (it != _peers.end()) it->second.latency_ms = ms;
}

bool Network::start_server(uint16_t port) {
    if (_server) return false;

    delete _client;
    _client = nullptr;

    _server = new Server();
    if (!_server->Start(port)) {
        delete _server;
        _server = nullptr;
        return false;
    }

    _server->SetOnClientConnected(on_client_connected);
    _server->SetOnClientDisconnected(on_client_disconnected);
    _server->SetOnMessageReceived(on_server_message);

    _peer_id = kServerPeerId;
    _peers.clear();

    printf("[Network] started server on :%u\n", port);
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
    printf("[Network] connecting to %s\n", address.c_str());
    return true;
}

void Network::disconnect() {
    delete _server; _server = nullptr;
    delete _client; _client = nullptr;

    _nid_map.clear();
    _entity_map.clear();
    _peers.clear();
    packed_ids().clear();
    Rpc::clear();

    _peer_id = kServerPeerId;
    _next_net_id = 1;
    _next_peer_id = 1;
    _seq_counter = 1;
}

void Network::assign(uint64_t net_id, entt::entity e) {
    _nid_map[net_id] = e;
    _entity_map[e]   = net_id;
}

entt::entity Network::resolve(uint64_t net_id) {
    auto it = _nid_map.find(net_id);
    return it == _nid_map.end() ? entt::null : it->second;
}

uint64_t Network::resolve(entt::entity e) {
    auto it = _entity_map.find(e);
    return it == _entity_map.end() ? 0u : it->second;
}

void Network::remove(uint64_t net_id) {
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
    auto* a = reg.try_get<me::cmp::net::Authority>(e);
    if (!a) return true;
    return a->id == _peer_id;
}

uint32_t Network::next_net_id()  { return _next_net_id++; }
PeerId Network::next_peer_id() { return _next_peer_id++; }

const std::unordered_map<PeerId, PeerInfo>& Network::peers() { return _peers; }

bool Network::peer(PeerId id, PeerInfo& out) {
    auto it = _peers.find(id);
    if (it == _peers.end()) return false;
    out = it->second;
    return true;
}

void Network::remove_peer(PeerId id) { _peers.erase(id); }


void Network::handle_message(
    ConnectionID from_conn,
    const uint8_t* data,
    uint32_t size
) {
    BinaryReader r(data, size);

    Envelope env{};
    if (!read_envelope(r, env)) return;

    const bool on_server = (_server != nullptr);
    const PeerId sender = on_server ? peer_of_conn(from_conn) : kServerPeerId;

    switch (env.type) {
        case MsgType::Handshake: {
            if (on_server) {
                uint32_t proto = 0;
                if (!r.read(proto)) return;
                if (proto != kProtocolVersion) {
                    std::printf("[Network][Server] protocol mismatch conn=%u\n", from_conn);
                    _server->Kick(from_conn, "protocol mismatch");
                    return;
                }

                PeerId pid = next_peer_id();
                _peers[pid] = PeerInfo{ from_conn, "", 0.f };

                send(pid, MsgType::Handshake, SendFlag::Reliable,
                    [pid](BinaryWriter& w) {
                        w.write(pid);
                        w.write(kProtocolVersion);
                    });

                send(MsgType::PeerConnected, SendFlag::Reliable,
                    [pid](BinaryWriter& w) { w.write(pid); }, pid);

                for (auto& [id, info] : _peers) {
                    if (id == pid) continue;
                    send(pid, MsgType::PeerConnected, SendFlag::Reliable,
                        [id](BinaryWriter& w) { w.write(id); });
                }

                for (auto& [nid, packed_id_str] : packed_ids()) {
                    auto e = resolve(nid);
                    if (e == entt::null) continue;
                    auto& reg = MainLoop::get_registry();
                    auto* a = reg.try_get<me::cmp::net::Authority>(e);
                    PeerId owner = a ? a->id : kServerPeerId;
                    send(pid, MsgType::EntitySpawn, SendFlag::Reliable,
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
                uint32_t pid = 0, proto = 0;
                if (!r.read(pid))   return;
                if (!r.read(proto)) return;
                if (proto != kProtocolVersion) {
                    std::printf("[Network][Client] protocol mismatch\n");
                    disconnect();
                    return;
                }
                _peer_id = pid;
                std::printf("[Network][Client] assigned peer id %u\n", pid);
            }
            break;
        }

        case MsgType::PeerConnected: {
            if (on_server) return;
            uint32_t pid = 0;
            if (!r.read(pid)) return;
            _peers.emplace(pid, PeerInfo{});
            std::printf("[Network][Client] peer %u connected\n", pid);
            break;
        }

        case MsgType::PeerDisconnected: {
            if (on_server) return;
            uint32_t pid = 0;
            if (!r.read(pid)) return;
            _peers.erase(pid);
            std::printf("[Network][Client] peer %u left\n", pid);
            break;
        }

        case MsgType::EntitySpawn: {
            if (on_server) return;
            uint32_t nid = 0, owner = 0;
            std::string packed_id;
            if (!r.read(nid))       return;
            if (!r.read(owner))     return;
            if (!r.read(packed_id)) return;

            if (resolve(nid) != entt::null) {
                auto& reg = MainLoop::get_registry();
                auto e = resolve(nid);
                if (reg.valid(e))
                    reg.emplace_or_replace<me::cmp::net::Authority>(e, me::cmp::net::Authority{owner});
                return;
            }

            auto& reg = MainLoop::get_registry();
            auto e = PackedEntity::spawn(packed_id, reg);
            if (e == entt::null) {
                std::printf("[Network][Client] EntitySpawn: unknown '%s'\n", packed_id.c_str());
                return;
            }

            reg.emplace_or_replace<me::cmp::net::NetId>(e).value = nid;
            reg.emplace_or_replace<me::cmp::net::Authority>(e).id = owner;
            assign(nid, e);
            break;
        }

        case MsgType::EntityDespawn: {
            if (on_server) return;
            uint32_t nid = 0;
            if (!r.read(nid)) return;
            auto e = resolve(nid);
            if (e == entt::null) return;
            auto& reg = MainLoop::get_registry();
            if (reg.valid(e)) reg.destroy(e);
            remove(nid);
            break;
        }

        case MsgType::EntitySnapshot: {
            uint32_t count = 0;
            if (!r.read(count)) return;

            auto& reg = MainLoop::get_registry();
            for (uint32_t i = 0; i < count; ++i) {
                uint32_t nid  = 0;
                uint64_t mask = 0;
                if (!r.read(nid))  return;
                if (!r.read(mask)) return;

                auto e = resolve(nid);

                if (e == entt::null || !reg.valid(e)) {
                    if (!Replication::skip_components(r, mask)) return;
                    continue;
                }

                if (on_server) {
                    auto* a = reg.try_get<me::cmp::net::Authority>(e);
                    if (!a || a->id != sender) {
                        if (!Replication::skip_components(r, mask)) return;
                        continue;
                    }
                }

                if (!on_server && is_authority(e)) {
                    if (!Replication::skip_components(r, mask)) return;
                    continue;
                }

                if (!Replication::read_components(r, reg, e, mask)) return;
            }
            break;
        }

        case MsgType::EntityAuthority: {
            uint32_t nid = 0, new_owner = 0;
            if (!r.read(nid))       return;
            if (!r.read(new_owner)) return;

            auto e = resolve(nid);
            if (e == entt::null) return;

            auto& reg = MainLoop::get_registry();
            if (reg.valid(e))
                reg.emplace_or_replace<me::cmp::net::Authority>(e).id = new_owner;
            break;
        }

        case MsgType::RpcCall: {
            Rpc::handle_call(sender, on_server, r);
            break;
        }
        case MsgType::RpcReply: {
            Rpc::handle_reply(r);
            break;
        }

        case MsgType::Ping: {
            uint32_t stamp = 0;
            if (!r.read(stamp)) return;

            auto write_pong = [stamp](BinaryWriter& w) { w.write(stamp); };

            if (on_server) {
                if (sender != kInvalidPeerId)
                    send(sender, MsgType::Pong, SendFlag::UnreliableNoNagle, write_pong);
            } else {
                send(kServerPeerId, MsgType::Pong, SendFlag::UnreliableNoNagle, write_pong);
            }
            break;
        }

        case MsgType::Pong: {
            uint32_t stamp = 0;
            if (!r.read(stamp)) return;
            uint32_t rtt = now_ms() - stamp;
            if (on_server) {
                if (sender != kInvalidPeerId)
                    set_peer_latency(sender, static_cast<float>(rtt));
            } else {
                set_peer_latency(kServerPeerId, static_cast<float>(rtt));
            }
            break;
        }

        default: break;
    }
}

}