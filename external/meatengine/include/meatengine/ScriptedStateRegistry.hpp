#pragma once

#include <string>
#include <unordered_map>
#include <sol/sol.hpp>

namespace me {

class ScriptedStateRegistry {
public:
    ScriptedStateRegistry() = delete;

    static void register_ss(const std::string& id, sol::table callbacks);
    static void change_to(const std::string& id);
    static bool exists(const std::string& id);

private:
    struct Entry {
        sol::protected_function on_enter;
        sol::protected_function on_exit;
        sol::protected_function handle_event;
    };

    static std::unordered_map<std::string, Entry>& registry();
};

} // namespace me