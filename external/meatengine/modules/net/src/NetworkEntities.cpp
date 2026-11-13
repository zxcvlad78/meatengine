#include <meatengine/net/Network.hpp>
#include <meatengine/net/Replication.hpp>

#include <meatengine/app/MainLoop.hpp>
#include <meatengine/ecs/PackedEntity.hpp>

#include <meatengine/net/cmp/Authority.hpp>
#include <meatengine/net/cmp/NetId.hpp>

#include "NetworkInternal.hpp"

namespace me::net {

namespace detail {

std::unordered_map<std::uint32_t, std::string>& packed_ids() {
    static std::unordered_map<std::uint32_t, std::string> m;
    return m;
}

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
    detail::packed_ids().erase(static_cast<std::uint32_t>(net_id));
}

void Network::remove(entt::entity e) {
    auto it = _entity_map.find(e);
    if (it == _entity_map.end()) return;
    detail::packed_ids().erase(static_cast<std::uint32_t>(it->second));
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
    detail::packed_ids()[nid] = packed_id;

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

}
