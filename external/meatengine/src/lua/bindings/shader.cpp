#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::Shader>("Shader", sol::no_constructor);
    
    sol::table rl = lua["ResourceLoader"];
    rl.set_function("load_Shader",
        [](const std::string& vs, const std::string& fs) -> me::Shader& {
            auto res = me::ResourceLoader::load<me::Shader>(vs, fs);
            auto h = res.handle();
            if (!h) throw std::runtime_error("load_Shader failed: " + vs + " / " + fs);
            return *h;
        }
    );

    rl.set_function("get_Shader",
        [](const std::string& vs, const std::string& fs) -> me::Shader* {
            auto res = me::ResourceLoader::get<me::Shader>(vs, fs);
            auto h = res.handle();
            return h ? h.get() : nullptr;
        }
    );
}

ME_LUA_BINDING(
    "engine:Shader",
    static_cast<unsigned>(me::lua::BindPriority::Resource),
    &bind
)