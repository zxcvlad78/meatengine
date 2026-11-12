#include <iostream>
#include <string>

#include <meatengine/app/MainLoop.hpp>
#include <meatengine/debug/Console.hpp>
#include <meatengine/scripting/ScriptingServer.hpp>

namespace {

    int lua_print(lua_State* L) {
        int n = lua_gettop(L);
        std::string result;
        for (int i = 1; i <= n; ++i) {
            size_t len = 0;
            const char* s = luaL_tolstring(L, i, &len);
            if (i > 1) result += '\t';
            result.append(s, len);
            lua_pop(L, 1);
            std::cout << s << std::endl;
        }
        me::debug::Console::get_instance().print(result);
        return 0;
    }

    struct LuaPrintHooks {
        LuaPrintHooks() {
            me::MainLoop::add_on_init([](sf::RenderWindow&) {
                me::ScriptingServer::lua().set_function("print", &lua_print);
            });
        }
    };

    LuaPrintHooks g_lua_print_hooks;
}
