#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <meatengine/net/Network.hpp>
#include <meatengine/net/Replication.hpp>

#include <meatengine/cmp/Transform.hpp>
#include <meatengine/cmp/net/NetId.hpp>
#include <meatengine/cmp/net/Authority.hpp>

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
    cmp::Transform last;
    float last_sent = 0.f;
};

std::unordered_map<entt::entity, Entry>& cache() {
    static std::unordered_map<entt::entity, Entry> c;
    return c;
}

bool transform_changed(const cmp::Transform& a, const cmp::Transform& b) {
    constexpr float eps = 1e-4f;
    return std::abs(a.position.x - b.position.x) > eps
        || std::abs(a.position.y - b.position.y) > eps
        || std::abs(a.rotation.asRadians() - b.rotation.asRadians()) > eps
        || std::abs(a.scale.x - b.scale.x) > eps
        || std::abs(a.scale.y - b.scale.y) > eps;
}

void server_tick() {
    auto& reg = MainLoop::get_registry();
    auto& c = cache();
    const float now = Network::now_ms() * 0.001f;

    std::vector<entt::entity> candidates;
    candidates.reserve(64);

    for (auto [e, t, nid] : reg.view<cmp::Transform, cmp::net::NetId>().each()) {
        auto it = c.find(e);
        if (it == c.end()) {
            c[e] = Entry{ t, now };
            candidates.push_back(e);
            continue;
        }
        bool changed = transform_changed(it->second.last, t);
        bool keyframe = (now - it->second.last_sent) >= KEYFRAME_TIME;
        if (changed || keyframe) {
            it->second.last = t;
            it->second.last_sent = now;
            candidates.push_back(e);
        }
    }

    if (candidates.empty()) return;

    for (std::size_t base = 0; base < candidates.size(); base += BATCH) {
        const std::size_t n = std::min(BATCH, candidates.size() - base);

        Network::send(MsgType::EntitySnapshot, SendFlag::Unreliable,
            [&](BinaryWriter& w) {
                w.write(static_cast<uint32_t>(n));
                for (std::size_t i = 0; i < n; ++i) {
                    auto e = candidates[base + i];
                    auto nid = reg.get<cmp::net::NetId>(e).value;
                    auto mask = Replication::mask_for(reg, e);
                    w.write(nid);
                    w.write(mask);
                    Replication::write_components(w, reg, e, mask);
                }
            });
    }

    for (auto it = c.begin(); it != c.end(); ) {
        if (!reg.valid(it->first)) it = c.erase(it);
        else ++it;
    }
}

void client_tick() {
    auto& reg = MainLoop::get_registry();
    auto& c = cache();
    const float now = Network::now_ms() * 0.001f;

    std::vector<entt::entity> to_send;
    to_send.reserve(16);

    for (auto [e, t, nid] : reg.view<cmp::Transform, cmp::net::NetId>().each()) {
        if (!Network::is_authority(e)) continue;

        auto it = c.find(e);
        if (it == c.end()) {
            c[e] = Entry{ t, now };
            to_send.push_back(e);
            continue;
        }
        bool changed = transform_changed(it->second.last, t);
        bool keyframe = (now - it->second.last_sent) >= KEYFRAME_TIME;
        if (changed || keyframe) {
            it->second.last = t;
            it->second.last_sent = now;
            to_send.push_back(e);
        }
    }

    if (to_send.empty()) return;

    Network::send(kServerPeerId, MsgType::EntitySnapshot, SendFlag::Unreliable,
        [&](BinaryWriter& w) {
            w.write(static_cast<uint32_t>(to_send.size()));
            for (auto e : to_send) {
                auto nid  = reg.get<cmp::net::NetId>(e).value;
                auto mask = Replication::mask_for(reg, e);
                w.write(nid);
                w.write(mask);
                Replication::write_components(w, reg, e, mask);
            }
        });

    for (auto it = c.begin(); it != c.end(); ) {
        if (!reg.valid(it->first)) it = c.erase(it);
        else ++it;
    }
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