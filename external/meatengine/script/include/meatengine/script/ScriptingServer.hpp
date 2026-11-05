#pragma once

#include <sol/sol.hpp>
#include <string>
#include <string_view>
#include <meatengine/ResourceLoader.hpp>
#include <meatengine/Resources.hpp>

#include <entt/entt.hpp>

namespace me {
    class ScriptingServer {
    private:
        static sol::state lua_state;
    public:
        ScriptingServer() = delete;

		static bool run_file(std::string_view path);
		static bool run_string(std::string_view code);

		static sol::state& lua();
    };
} // namespace me