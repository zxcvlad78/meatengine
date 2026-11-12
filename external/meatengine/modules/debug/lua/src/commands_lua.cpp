#include <string>
#include <vector>

#include <meatengine/debug/Console.hpp>
#include <meatengine/scripting/ScriptingServer.hpp>

namespace {

    void run_lua_command(const std::vector<std::string>& args) {
        try {
            const std::string& text = args.at(0);
            if (text.starts_with("path::")) {
                std::string path = text;
                path.erase(0, 6);
                me::ScriptingServer::run_file(path);
            }
            else {
                std::string total_string;
                for (std::string s : args) total_string.append(s + " ");

                me::ScriptingServer::run_string(total_string);
            }
        } catch (const std::exception& e) {
            me::debug::Console::get_instance().print_error(e.what());
        }
    }

    struct LuaConsoleCommand {
        LuaConsoleCommand() {
            me::debug::Console::get_instance().register_command(
                "lua",
                &run_lua_command,
                "Run lua script",
                "lua <script>"
            );
        }
    };

    LuaConsoleCommand g_lua_console_command;
}
