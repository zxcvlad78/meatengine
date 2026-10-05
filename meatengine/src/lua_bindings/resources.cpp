#include "meatengine/lua_bindings/components.hpp"
#include "meatengine/lua_bindings/common.hpp"
#include "meatengine/Resources.hpp"
#include "meatengine/ResourceLoader.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace me::lua_bindings {

void init_resources(sol::state& lua) {
    sol::table rl = lua.create_table();
    lua["ResourceLoader"] = rl;

    bind_resource<me::Font>        (rl, "Font");
    bind_resource<me::Texture>     (rl, "Texture");
    bind_resource<me::SoundBuffer> (rl, "SoundBuffer");
    bind_resource<me::SpriteSheet> (rl, "SpriteSheet");
    bind_resource<me::TileSet>     (rl, "TileSet");
    bind_resource<me::StyleBox>    (rl, "StyleBox");

    rl.set_function("load_shader",
        [](const std::string& vs, const std::string& fs) -> me::Shader& {
            auto res = me::ResourceLoader::load<me::Shader>(vs, fs);
            auto h = res.handle();
            if (!h) throw std::runtime_error("load_shader failed: " + vs + " / " + fs);
            return *h;
        });

    rl.set_function("get_shader",
        [](const std::string& vs, const std::string& fs) -> me::Shader* {
            auto res = me::ResourceLoader::get<me::Shader>(vs, fs);
            auto h = res.handle();
            return h ? h.get() : nullptr;
        });

    lua.new_usertype<me::Font>("Font", sol::no_constructor);

    lua.new_usertype<me::Texture>("Texture",
        sol::no_constructor,
        "size", sol::property(
            [](me::Texture& t) { return t.res.getSize(); }
        ),
        "smooth", sol::property(
            [](me::Texture& t) { return t.res.isSmooth(); },
            [](me::Texture& t, bool v) { t.res.setSmooth(v); }
        ),
        "repeated", sol::property(
            [](me::Texture& t) { return t.res.isRepeated(); },
            [](me::Texture& t, bool v) { t.res.setRepeated(v); }
        )
    );

    lua.new_usertype<me::SoundBuffer>("SoundBuffer",
        sol::no_constructor,
        "duration", sol::property(
            [](me::SoundBuffer& sb) { return sb.res.getDuration().asSeconds(); }
        )
    );

    lua.new_usertype<me::Shader>("Shader", sol::no_constructor);

    lua.new_usertype<me::Animation::FrameData>("AnimationFrame",
        sol::constructors<me::Animation::FrameData()>(),
        "x", &me::Animation::FrameData::x,
        "y", &me::Animation::FrameData::y,
        "w", &me::Animation::FrameData::w,
        "h", &me::Animation::FrameData::h
    );

    lua.new_usertype<me::Animation>("Animation",
        sol::constructors<me::Animation()>(),
        "name",       &me::Animation::name,
        "fps",        &me::Animation::fps,
        "is_looping", &me::Animation::is_looping,
        "frames",     &me::Animation::frames,
        "duration",   sol::property(
            [](me::Animation& a) { return a.duration(); }
        )
    );

    lua.new_usertype<me::SpriteSheet>("SpriteSheet",
        sol::no_constructor,
        "atlas_width",  &me::SpriteSheet::atlas_width,
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

    lua.new_usertype<me::TileSet>("TileSet",
        sol::no_constructor,
        "tile_size",     &me::TileSet::tile_size,
        "y_sort_origin", &me::TileSet::y_sort_origin,
        "size",          sol::property(
            [](me::TileSet& ts) { return ts.size(); }
        )
    );

    lua.new_usertype<me::StyleBox>("StyleBox",
        sol::no_constructor,

        "get_color",
            [](me::StyleBox& sb, const std::string& k, sol::optional<sf::Color> def) {
                return sb.get_value<sf::Color>(k, def.value_or(sf::Color::White));
            },
        "get_float",
            [](me::StyleBox& sb, const std::string& k, sol::optional<float> def) {
                return sb.get_value<float>(k, def.value_or(0.f));
            },
        "get_int",
            [](me::StyleBox& sb, const std::string& k, sol::optional<int> def) {
                return sb.get_value<int>(k, def.value_or(0));
            },
        "get_bool",
            [](me::StyleBox& sb, const std::string& k, sol::optional<bool> def) {
                return sb.get_value<bool>(k, def.value_or(false));
            },
        "get_string",
            [](me::StyleBox& sb, const std::string& k, sol::optional<std::string> def) {
                return sb.get_value<std::string>(k, def.value_or(""));
            },

        "set_color",
            [](me::StyleBox& sb, const std::string& k, sf::Color v) {
                sb.set_value<sf::Color>(k, v);
            },
        "set_float",
            [](me::StyleBox& sb, const std::string& k, float v) {
                sb.set_value<float>(k, v);
            },
        "set_int",
            [](me::StyleBox& sb, const std::string& k, int v) {
                sb.set_value<int>(k, v);
            },
        "set_bool",
            [](me::StyleBox& sb, const std::string& k, bool v) {
                sb.set_value<bool>(k, v);
            },
        "set_string",
            [](me::StyleBox& sb, const std::string& k, const std::string& v) {
                sb.set_value<std::string>(k, v);
            }
    );

    lua.set_function("load_font",
        [](const std::string& path) -> me::Font& {
            auto res = me::ResourceLoader::load<me::Font>(path);
            auto h = res.handle();
            if (!h) throw std::runtime_error("load_font failed: " + path);
            return *h;
        });

    lua.set_function("load_texture",
        [](const std::string& path) -> me::Texture& {
            auto res = me::ResourceLoader::load<me::Texture>(path);
            auto h = res.handle();
            if (!h) throw std::runtime_error("load_texture failed: " + path);
            return *h;
        });

    lua.set_function("load_sound_buffer",
        [](const std::string& path) -> me::SoundBuffer& {
            auto res = me::ResourceLoader::load<me::SoundBuffer>(path);
            auto h = res.handle();
            if (!h) throw std::runtime_error("load_sound_buffer failed: " + path);
            return *h;
        });

    lua.set_function("load_spritesheet",
        [](const std::string& path) -> me::SpriteSheet& {
            auto res = me::ResourceLoader::load<me::SpriteSheet>(path);
            auto h = res.handle();
            if (!h) throw std::runtime_error("load_spritesheet failed: " + path);
            return *h;
        });

    lua.set_function("load_tileset",
        [](const std::string& path) -> me::TileSet& {
            auto res = me::ResourceLoader::load<me::TileSet>(path);
            auto h = res.handle();
            if (!h) throw std::runtime_error("load_tileset failed: " + path);
            return *h;
        });

    lua.set_function("load_stylebox",
        [](const std::string& path) -> me::StyleBox& {
            auto res = me::ResourceLoader::load<me::StyleBox>(path);
            auto h = res.handle();
            if (!h) throw std::runtime_error("load_stylebox failed: " + path);
            return *h;
        });
}

} // namespace me::lua_bindings