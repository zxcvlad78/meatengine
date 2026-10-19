#include <meatengine/net/Replication.hpp>

namespace me::net {

std::vector<ReplicatedComponent>& Replication::components() {
    static std::vector<ReplicatedComponent> c;
    return c;
}

std::uint64_t Replication::mask_for(entt::registry& r, entt::entity e) {
    std::uint64_t m = 0;
    auto& comps = components();
    for (std::size_t i = 0; i < comps.size() && i < 64; ++i)
        if (comps[i].has(r, e))
            m |= (std::uint64_t{1} << i);
    return m;
}

void Replication::write_components(
    BinaryWriter& w,
    entt::registry& r,
    entt::entity e, std::uint64_t mask
) {
    auto& comps = components();
    for (std::size_t i = 0; i < comps.size() && i < 64; ++i)
        if (mask & (std::uint64_t{1} << i))
            comps[i].write(w, r, e);
}

bool Replication::read_components(
    BinaryReader& r,
    entt::registry& reg,
    entt::entity e, std::uint64_t mask
) {
    auto& comps = components();
    for (std::size_t i = 0; i < comps.size() && i < 64; ++i) {
        if (!(mask & (std::uint64_t{1} << i))) continue;
        if (!comps[i].read(r, reg, e)) return false;
    }
    return true;
}

bool Replication::skip_components(BinaryReader& r, std::uint64_t mask) {
    auto& comps = components();
    for (std::size_t i = 0; i < comps.size() && i < 64; ++i) {
        if (!(mask & (std::uint64_t{1} << i))) continue;
        if (!comps[i].skip(r)) return false;
    }
    return true;
}

}