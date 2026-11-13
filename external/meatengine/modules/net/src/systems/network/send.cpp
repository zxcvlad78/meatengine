#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/net/Network.hpp>
#include <meatengine/net/Replication.hpp>

#include <meatengine/spatial/cmp/Transform.hpp>
#include <meatengine/net/cmp/NetId.hpp>
#include <meatengine/net/cmp/Authority.hpp>

#include <cmath>
#include <unordered_map>
#include <vector>

namespace {

using namespace me;
using namespace me::net;

constexpr float TICK_RATE = 1.f / 30.f;
constexpr float KEYFRAME_TIME = 1.f;
constexpr std::size_t BATCH = 32;

struct Entry {
    me::spatial::cmp::Transform last;
    float last_sent = 0.f;
};

struct Candidate {
    entt::entity entity;
    std::uint32_t net_id;
    std::uint64_t mask;
};

std::unordered_map<entt::entity, Entry>& cache() {
    static std::unordered_map<entt::entity, Entry> c;
    return c;
}

bool transform_changed(const me::spatial::cmp::Transform& a, const me::spatial::cmp::Transform& b) {
    constexpr float eps = 1e-4f;
    return std::abs(a.position.x - b.position.x) > eps
        || std::abs(a.position.y - b.position.y) > eps
        || std::abs(a.rotation.asRadians() - b.rotation.asRadians()) > eps
        || std::abs(a.scale.x - b.scale.x) > eps
        || std::abs(a.scale.y - b.scale.y) > eps;
}

void collect(std::vector<Candidate>& out, bool authority_only) {
    auto& reg = MainLoop::get_registry();
    auto& c = cache();
    const float now = Network::now_ms() * 0.001f;

    for (auto [e, t, nid] : reg.view<me::spatial::cmp::Transform, me::net::cmp::NetId>().each()) {
        if (authority_only && !Network::is_authority(e)) continue;

        auto it = c.find(e);
        if (it == c.end()) {
            c[e] = Entry{ t, now };
            out.push_back(Candidate{ e, nid.value, Replication::mask_for(reg, e) });
            continue;
        }

        const bool changed = transform_changed(it->second.last, t);
        const bool keyframe = (now - it->second.last_sent) >= KEYFRAME_TIME;
        if (changed || keyframe) {
            it->second.last = t;
            it->second.last_sent = now;
            out.push_back(Candidate{ e, nid.value, Replication::mask_for(reg, e) });
        }
    }
}

void prune() {
    auto& reg = MainLoop::get_registry();
    auto& c = cache();
    for (auto it = c.begin(); it != c.end(); ) {
        if (!reg.valid(it->first)) it = c.erase(it);
        else ++it;
    }
}

void server_tick() {
    std::vector<Candidate> candidates;
    candidates.reserve(64);
    collect(candidates, false);
    if (candidates.empty()) { prune(); return; }

    auto& reg = MainLoop::get_registry();

    for (std::size_t base = 0; base < candidates.size(); base += BATCH) {
        const std::size_t n = std::min(BATCH, candidates.size() - base);

        Network::send(MsgType::EntitySnapshot, SendFlag::Unreliable,
            [&](BinaryWriter& w) {
                w.write(static_cast<uint32_t>(n));
                for (std::size_t i = 0; i < n; ++i) {
                    const Candidate& cand = candidates[base + i];
                    w.write(cand.net_id);
                    w.write(cand.mask);
                    Replication::write_components(w, reg, cand.entity, cand.mask);
                }
            });
    }

    prune();
}

void client_tick() {
    std::vector<Candidate> to_send;
    to_send.reserve(16);
    collect(to_send, true);
    if (to_send.empty()) { prune(); return; }

    auto& reg = MainLoop::get_registry();

    Network::send(kServerPeerId, MsgType::EntitySnapshot, SendFlag::Unreliable,
        [&](BinaryWriter& w) {
            w.write(static_cast<uint32_t>(to_send.size()));
            for (const Candidate& cand : to_send) {
                w.write(cand.net_id);
                w.write(cand.mask);
                Replication::write_components(w, reg, cand.entity, cand.mask);
            }
        });

    prune();
}

void tick() {
    if (Network::is_server()) server_tick();
    else if (Network::is_client()) client_tick();
}

float s_accum = 0.f;

void run() {
    if (!Network::is_server() && !Network::is_client()) return;
    s_accum += MainLoop::get_dt();
    if (s_accum < TICK_RATE) return;
    s_accum = 0.f;
    tick();
}

}

ME_REGISTER_SYSTEM(
    "engine:network:send",
    me::SystemPhase::PostRender,
    100,
    run
)
