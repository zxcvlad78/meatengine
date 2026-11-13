#include <meatengine/net/Network.hpp>
#include <meatengine/net/Replication.hpp>
#include <meatengine/net/Rpc.hpp>

#include <meatengine/app/MainLoop.hpp>
#include <meatengine/ecs/PackedEntity.hpp>

#include <meatengine/net/cmp/Authority.hpp>
#include <meatengine/net/cmp/NetId.hpp>

#include <cstdio>

#include "NetworkInternal.hpp"

namespace me::net::detail {

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

namespace me::net {

void Network::handle_message(PeerId sender, const std::uint8_t* data, std::uint32_t size) {
    _stats.messages_received++;
    _stats.bytes_received += size;

    BinaryReader r(data, size);

    Envelope env{};
    if (!read_envelope(r, env)) return;

    detail::dispatch_message(env.type, sender, r);
}

}
