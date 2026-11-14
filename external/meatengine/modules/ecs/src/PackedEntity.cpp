#include <meatengine/ecs/PackedEntity.hpp>
#include <stdexcept>
#include <iostream>

namespace me {

std::unordered_map<std::string, PackedEntity::SpawnFn>& PackedEntity::registry() {
    static std::unordered_map<std::string, SpawnFn> r;
    return r;
}

void PackedEntity::register_pe(const std::string& id, SpawnFn fn) {
    if (id.empty())
        throw std::runtime_error("[PackedEntity] empty id");

    if (!fn)
        throw std::runtime_error("[PackedEntity] invalid function for '" + id + "'");

    auto& reg = registry();
    if (reg.count(id)) {
        reg[id] = std::move(fn);
        return;
    }

    reg.emplace(id, std::move(fn));
}


bool PackedEntity::exists(const std::string& id) {
    return registry().count(id) > 0;
}

void PackedEntity::clear() {
    registry().clear();
}

entt::entity PackedEntity::spawn(const std::string& id, entt::registry& reg) {
    auto it = registry().find(id);
    if (it == registry().end()) {
        std::cerr << "[PackedEntity] unknown '" << id << "'\n";
        return entt::null;
    }

    return it->second(reg);
}

} // namespace me
