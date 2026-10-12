#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::SpriteSheet>("SpriteSheet",
        sol::no_constructor,
        "atlas_width", &me::SpriteSheet::atlas_width,
        "atlas_height", &me::SpriteSheet::atlas_height,
        "get_animation",
            [](me::SpriteSheet& ss, const std::string& name)
                -> me::Animation* {
                auto it = ss.animations.find(name);
                if (it == ss.animations.end()) return nullptr;
                return &it->second;
            },
        "animation_names",
            [](me::SpriteSheet& ss) {
                std::vector<std::string> names;
                names.reserve(ss.animations.size());
                for (auto& [k, v] : ss.animations) names.push_back(k);
                return names;
            }
    );

    sol::table rl = lua["ResourceLoader"];
    me::lua::register_resource<me::SpriteSheet>(rl, "SpriteSheet");
}

ME_LUA_BINDING(
    "engine:SpriteSheet",
    static_cast<unsigned>(me::lua::BindPriority::Resource),
    &bind
)