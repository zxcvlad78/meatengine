#include <meatengine/ScriptingServer.hpp>
#include <meatengine/lua_bindings/common.hpp>

#include <iostream>

namespace me {
    sol::state ScriptingServer::lua_state;

    static void ensure_initialized() {
        static bool inited = false;
        if (inited) return;
        inited = true;

        auto& lua = ScriptingServer::lua();

        lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::table,
            sol::lib::string,
            sol::lib::math,
            sol::lib::io,
            sol::lib::os
        );

        lua_bindings::init(lua);
    }

    bool ScriptingServer::run_file(std::string_view path) {
        ensure_initialized();
        try {
            lua_state.safe_script_file(std::string(path));
            return true;
        } catch (const sol::error& e) {
            std::cerr << "[ScriptingServer] " << e.what() << '\n';
            return false;
        }
    }

    bool ScriptingServer::run_string(std::string_view code) {
        ensure_initialized();
        try {
            lua_state.safe_script(std::string(code));
            return true;
        } catch (const sol::error& e) {
            std::cerr << "[ScriptingServer] " << e.what() << '\n';
            return false;
        }
    }

    sol::state& ScriptingServer::lua() {
        ensure_initialized();
        return lua_state;
    }
} // namespace me