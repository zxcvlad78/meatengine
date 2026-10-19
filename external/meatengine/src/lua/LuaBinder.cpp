#include <meatengine/lua/LuaBinder.hpp>
#include <algorithm>
#include <iostream>

namespace me::lua {

std::vector<BindingEntry>& binding_list() {
    static std::vector<BindingEntry> l;
    return l;
}

BindingAutoReg::BindingAutoReg(
    std::string name, 
    unsigned int priority,
    std::function<void(sol::state&)> fn
) {
    binding_list().push_back({std::move(name), priority, std::move(fn)});
}

void bind(sol::state& lua) {
    auto& l = binding_list();

    std::stable_sort(
        l.begin(), 
        l.end(),
        [](const BindingEntry& a, const BindingEntry& b) {
            if (a.priority != b.priority) return a.priority < b.priority;
            return a.name < b.name;
        }
    );

    std::unordered_set<std::string> seen;
    for (auto& e : l) {
        if (!seen.insert(e.name).second) continue;
        try { e.func(lua); }
        catch (const std::exception& ex) {
            std::cerr << "[LuaBinder] '" << e.name << "': " << ex.what() << "\n";
        }
    }
}

} // namespace me::lua