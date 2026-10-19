#include <meatengine/PackedEntity.hpp>
#include <stdexcept>
#include <iostream>

namespace me {

std::unordered_map<std::string, sol::protected_function>& PackedEntity::registry() {
    static std::unordered_map<std::string, sol::protected_function> r;
    return r;
}

void PackedEntity::register_pe(const std::string& id, sol::protected_function fn) {
    if (id.empty())
        throw std::runtime_error("[PackedEntity] empty id");

    if (!fn.valid())
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

entt::entity PackedEntity::spawn(const std::string& id, entt::registry& reg) {
    auto it = registry().find(id);
    if (it == registry().end()) {
        std::cerr << "[PackedEntity] unknown '" << id << "'\n";
        return entt::null;
    }

    sol::protected_function fn = it->second;

    sol::protected_function_result r = fn(reg);
    if (!r.valid()) {
        sol::error err = r;
        std::cerr << "[PackedEntity] '" << id << "' failed: " << err.what() << "\n";
        return entt::null;
    }

    if (r.get_type() == sol::type::nil) {
        std::cerr << "[PackedEntity] '" << id << "' returned nil\n";
        return entt::null;
    }

    try {
        return r.get<entt::entity>();
    } catch (const std::exception& e) {
        std::cerr << "[PackedEntity] '" << id << "' bad return: " << e.what() << "\n";
        return entt::null;
    }
}

} // namespace me