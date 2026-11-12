#pragma once

#include <entt/entt.hpp>

#include <functional>
#include <string>
#include <unordered_map>

namespace me {

// PackedEntity() = delete;
// надо будет потом сделать крутой универсамовский xD Registry как в "minekraf't"
class PackedEntity {
public:
    using SpawnFn = std::function<entt::entity(entt::registry&)>;

    PackedEntity() = delete;

    static void register_pe(const std::string& id, SpawnFn fn);
    static entt::entity spawn(const std::string& id, entt::registry& reg);
    static bool exists(const std::string& id);

private:
    static std::unordered_map<std::string, SpawnFn>& registry();
};

} // namespace me
