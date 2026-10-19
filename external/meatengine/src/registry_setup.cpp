#include <meatengine/registry_setup.hpp>
#include <iostream>

namespace me::registry_setup {

std::vector<SetupEntry>& list() {
    static std::vector<SetupEntry> l;
    return l;
}

void run(entt::registry& reg) {
    for (auto& e : list()) {
        try {
            e.func(reg);
        } catch (const std::exception& ex) {
            std::cerr << "[registry_setup] '" << e.name << "' " << ex.what() << "\n";
        }
    }
}

} // namespace me