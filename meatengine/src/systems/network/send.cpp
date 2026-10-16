#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/GlobalTransform.hpp>
#include <meatengine/cmp/net/NetId.hpp>
#include <meatengine/meatnet/Network.hpp>

#include <unordered_map>
#include <unordered_set>

namespace {

using namespace me;

constexpr float SEND_INTERVAL = 1.f / 30.f;
static float s_accum = 0.f;

std::unordered_map<entt::entity, sf::Transform>& last_sent() {
    static std::unordered_map<entt::entity, sf::Transform> m;
    return m;
}

bool transform_changed(const sf::Transform& a, const sf::Transform& b) {
    const float* ma = a.getMatrix();
    const float* mb = b.getMatrix();
    
    constexpr int idx[] = {0, 4, 8, 1, 5, 9, 2, 6, 10};
    for (int i : idx)
        if (std::abs(ma[i] - mb[i]) > 1e-4f) return true;
    return false;
}

void run() {
    auto& reg = MainLoop::get_registry();
    float dt = MainLoop::get_dt();

    s_accum += dt;
    if (s_accum < SEND_INTERVAL) return;
    s_accum = 0.f;

    auto* srv = net::Network::server();
    if (!srv) return;

    auto& cache = last_sent();

    for (auto [e, gt, id] : reg.view<cmp::GlobalTransform, cmp::net::NetId>().each()) {
        auto it = cache.find(e);
        if (it != cache.end() && !transform_changed(it->second, gt.transform)) {
            continue;
        }
        cache[e] = gt.transform;

        auto bytes = me::cmp::serialize(gt);

        srv->Broadcast(
            &bytes,
            bytes.size(),
            k_HSteamNetConnection_Invalid,
            false
        );
    }

    if (cache.size() > reg.storage<entt::entity>().size()) {
        for (auto it = cache.begin(); it != cache.end(); ) {
            if (!reg.valid(it->first)) it = cache.erase(it);
            else ++it;
        }
    }
}

}

ME_REGISTER_SYSTEM(
    "engine:network:send",
    me::SystemPhase::PostRender,
    100,
    run
)