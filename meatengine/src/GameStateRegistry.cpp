#include <meatengine/GameStateRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <iostream>

namespace me {

std::unordered_map<std::string, GameStateRegistry::Factory>&
GameStateRegistry::factories() {
    static std::unordered_map<std::string, Factory> f;
    return f;
}

void GameStateRegistry::register_gs(const std::string& id, Factory factory) {
    if (id.empty()) {
        std::cerr << "[GameStateRegistry] empty id\n";
        return;
    }
    factories()[id] = std::move(factory);
}

bool GameStateRegistry::exists(const std::string& id) {
    return factories().count(id) > 0;
}

std::vector<std::string> GameStateRegistry::list() {
    std::vector<std::string> out;
    out.reserve(factories().size());
    for (auto& [k, _] : factories()) out.push_back(k);
    return out;
}

void GameStateRegistry::change_to(const std::string& id) {
    auto it = factories().find(id);
    if (it == factories().end()) {
        std::cerr << "[GameStateRegistry] unknown state '" << id << "'\n";
        return;
    }
    MainLoop::change_state(it->second());
}

} // namespace me