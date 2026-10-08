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
    take(e.update, "update");
    take(e.update_deferred, "update_deferred");
    take(e.render, "render");
    take(e.render_deferred, "render_deferred");
    take(e.render_default_view, "render_default_view");
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
    s->set_update(it->second.update);
    s->set_update_deferred(it->second.update_deferred);
    s->set_render(it->second.render);
    s->set_render_deferred(it->second.render_deferred);
    s->set_render_default_view(it->second.render_default_view);
    s->set_handle_event(it->second.handle_event);

    MainLoop::change_state(std::move(s));
}

} // namespace me