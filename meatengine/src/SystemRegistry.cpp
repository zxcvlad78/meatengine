#include <meatengine/SystemRegistry.hpp>

#include <iostream>
#include <vector>

namespace me {

SystemRegistry::PhaseBucket& SystemRegistry::bucket(SystemPhase phase) {
    return _buckets[static_cast<std::size_t>(phase)];
}

void SystemRegistry::emplace(SystemPhase phase, const std::string& id, int priority, std::function<void()> func) {
    if (id.empty()) {
        std::cerr << "[SystemRegistry] emplace: empty id\n";
        return;
    }
    if (phase == SystemPhase::Invalid) {
        std::cerr << "[SystemRegistry] emplace: invalid phase\n";
        return;
    }

    auto& bucket = _buckets[static_cast<std::size_t>(phase)];

    for (auto& e : bucket.entries) {
        if (e.id == id) {
            e.priority = priority;
            e.func = std::move(func);
            bucket.sorted = false;
            return;
        }
    }

    bucket.entries.push_back({id, priority, std::move(func)});
    bucket.sorted = false;
}

void SystemRegistry::remove(const std::string& id) {
    for (auto& b : _buckets) {
        b.entries.erase(
            std::remove_if(b.entries.begin(), b.entries.end(),
                [&](const Entry& e) { return e.id == id; }),
            b.entries.end());
    }
}

void SystemRegistry::remove(const std::string& id, SystemPhase phase) {
    if (id.empty()) return;
    if (phase == SystemPhase::Invalid) return;

    auto& b = bucket(phase);
    b.entries.erase(
        std::remove_if(
            b.entries.begin(),
            b.entries.end(),
            [&](const Entry& e) { return e.id == id; }
        ),
        b.entries.end()
    );
}

bool SystemRegistry::has(const std::string& id) {
    if (id.empty()) return false;

    for (auto& b : _buckets) {
        for (auto& e : b.entries) if (e.id == id) return true;
    }
    return false;
}

bool SystemRegistry::has(const std::string& id, SystemPhase phase) {
    if (phase == SystemPhase::Invalid) return false;

    for (auto& e : bucket(phase).entries)
        if (e.id == id) return true;
    
    return false;
}

std::vector<std::string> SystemRegistry::list() {
    std::vector<std::string> out;
    for (auto& b : _buckets) {
        for (auto& e : b.entries) out.push_back(e.id);
    }
    return out;
}

std::vector<std::string> SystemRegistry::list(SystemPhase phase) {
    std::vector<std::string> out;
    if (phase == SystemPhase::Invalid) return out;

    for (auto& e : bucket(phase).entries)
        out.push_back(e.id);
    
    return out;
}

void SystemRegistry::ensure_sorted(SystemPhase phase) {
    auto& b = bucket(phase);
    if (b.sorted) return;

    std::stable_sort(
        b.entries.begin(),
        b.entries.end(),
        [](const Entry& a, const Entry& b) { return a.priority < b.priority; }
    );

    b.sorted = true;
}

void SystemRegistry::run(SystemPhase phase) {
    if (phase == SystemPhase::Invalid) return;

    ensure_sorted(phase);

    for (auto& e : bucket(phase).entries) {
        if (e.func) e.func();
    }
}

} // namespace

