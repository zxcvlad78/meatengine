#pragma once

#include <string>
#include <unordered_map>
#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace me {

// надо будет потом сделать крутой универсамовский xD Registry как в "minekraf't" 
class PackedEntity {
public:
    PackedEntity() = delete;

    static void register_pe(const std::string& id, sol::protected_function fn);
    static entt::entity spawn(const std::string& id, entt::registry& reg);
    static bool exists(const std::string& id);

private:
    static std::unordered_map<std::string, sol::protected_function>& registry();
};

} // namespace me