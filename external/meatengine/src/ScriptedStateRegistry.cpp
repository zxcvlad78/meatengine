#include <meatengine/ScriptedStateRegistry.hpp>
#include <meatengine/ScriptedState.hpp>
#include <meatengine/MainLoop.hpp>

#include <iostream>

namespace me {

std::unordered_map<std::string, ScriptedStateRegistry::Entry>&
ScriptedStateRegistry::registry() {
    static std::unordered_map<std::string, Entry> r;
    return r;
}

void ScriptedStateRegistry::register_ss(const std::string& id, sol::table callbacks) {
    if (id.empty()) {
        std::cerr << "[ScriptedStateRegistry] empty id\n";
        return;
    }

    Entry e;

    
    auto take = [&](sol::protected_function& dst, const char* key) {
        sol::object o = callbacks[key];
        if (o.is<sol::protected_function>())
            dst = o.as<sol::protected_function>();
    };

    take(e.on_enter, "on_enter");
    take(e.on_exit, "on_exit");
    take(e.handle_event, "handle_event");

    registry()[id] = std::move(e);
}

bool ScriptedStateRegistry::exists(const std::string& id) {
    return registry().count(id) > 0;
}

void ScriptedStateRegistry::change_to(const std::string& id) {
    auto it = registry().find(id);
    if (it == registry().end()) {
        std::cerr << "[ScriptedStateRegistry] unknown state '" << id << "'\n";
        return;
    }

    auto s = std::make_unique<ScriptedState>(id);
    s->set_on_enter(it->second.on_enter);
    s->set_on_exit(it->second.on_exit);
    s->set_handle_event(it->second.handle_event);

    MainLoop::change_state(std::move(s));
}

} // namespace me