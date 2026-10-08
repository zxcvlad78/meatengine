#include "meatengine/lua_bindings/components.hpp"
#include "meatengine/lua_bindings/common.hpp"
#include "meatengine/meatengine.hpp"

#include <entt/entt.hpp>
#include <stdexcept>
#include <string>

namespace me::lua_bindings {

void init(sol::state& lua) {
    init_sfml(lua);
    init_resources(lua);
    init_common(lua);
}

void init_common(sol::state& lua) {
    auto reg_type = get_reg_type(lua);

    lua.new_usertype<sf::RenderWindow>("Window",
        sol::no_constructor,
        "size", [](sf::RenderWindow& w) { return w.getSize(); },
        "close", &sf::RenderWindow::close,
        "is_open", &sf::RenderWindow::isOpen,
        "set_title", [](sf::RenderWindow& w, const std::string& s) { w.setTitle(s); });

    sol::table ssr_table = lua.create_table();
    lua["ScriptedStateRegistry"] = ssr_table;

    ssr_table.set_function("register",
        [](const std::string& id, sol::table callbacks) {
            me::ScriptedStateRegistry::register_gs(id, callbacks);
        });

    ssr_table.set_function("change_to",
        [](const std::string& id) {
            me::ScriptedStateRegistry::change_to(id);
        });

    ssr_table.set_function("exists",
        [](const std::string& id) -> bool {
            return me::ScriptedStateRegistry::exists(id);
        });

    sol::table ml_table = lua.create_table();
    lua["MainLoop"] = ml_table;

    ml_table.set_function("get_registry",
        []() -> entt::registry& {
            return me::MainLoop::get_registry();
        });
    
    ml_table.set_function("get_fps",
        []() -> float {
            return me::MainLoop::get_fps();
        });
    
    ml_table.set_function("get_window_title",
        []() -> std::string {
            return me::MainLoop::get_window_title();
        });

    ml_table.set_function("set_window_title",
        [](const std::string& s) -> void {
            me::MainLoop::set_window_title(s);
        });

    ml_table.set_function("get_framerate_limit",
        []() -> float {
            return me::MainLoop::get_framerate_limit();
        });

    ml_table.set_function("set_framerate_limit",
        [](float v) -> void {
            me::MainLoop::set_framerate_limit(v);
        });
    

    sol::table pe_table = lua.create_table();
    lua["PackedEntity"] = pe_table;

    pe_table.set_function("register",
        [](const std::string& id, sol::protected_function fn) {
            me::PackedEntity::register_pe(id, fn);
        });

    pe_table.set_function("spawn",
        [](const std::string& id, entt::registry& reg) -> entt::entity {
            return me::PackedEntity::spawn(id, reg);
        });

    pe_table.set_function("exists",
        [](const std::string& id) -> bool {
            return me::PackedEntity::exists(id);
        });

    auto sprite_ut = lua.new_usertype<me::Sprite>("Sprite",
        sol::constructors<me::Sprite()>(),

        "texture", sol::property(
            [](me::Sprite& sp) -> me::Texture* { return sp.texture_ptr(); },
            [](me::Sprite& sp, me::Texture& t) { sp.set_texture(t); }
        ),

        "offset", &me::Sprite::offset,
        "center", &me::Sprite::center,

        "has_texture", &me::Sprite::has_texture,
        "has_sprite",  &me::Sprite::has_sprite,
        "clear_texture", &me::Sprite::clear_texture,

        "sf_sprite", sol::property(
            [](me::Sprite& sp) -> sf::Sprite* { return sp.sprite_ptr(); },
            [](me::Sprite&, sol::object) {
                throw std::runtime_error("sf_sprite is read-only");
            }
        )
    );

    auto sprite_animation_ut = lua.new_usertype<me::SpriteAnimation>("SpriteAnimation",
        sol::constructors<me::SpriteAnimation()>(),

        "spritesheet", sol::property(
            [](me::SpriteAnimation& sa) -> me::SpriteSheet* {
                return sa.spritesheet ? sa.spritesheet.operator->() : nullptr;
            },
            [](me::SpriteAnimation& sa, me::SpriteSheet& sheet) {
                auto h = me::ResourceLoader::find_handle(&sheet);
                if (!h) {
                    throw std::runtime_error(
                        "SpriteAnimation.spritesheet: SpriteSheet not from ResourceLoader "
                        "(load it via ResourceLoader.load_spritesheet first)");
                }
                sa.spritesheet = entt::resource<me::SpriteSheet>{h};
            }
        ),

        "is_playing",         &me::SpriteAnimation::is_playing,
        "current_frame_idx",  &me::SpriteAnimation::current_frame_idx,
        "time_accumulator",   &me::SpriteAnimation::time_accumulator,
        "next_anim",          &me::SpriteAnimation::next_anim,

        "play", [](me::SpriteAnimation& sa,
                const std::string& name,
                sol::optional<std::string> next) {
            return sa.play(name, next.value_or(""));
        }
    );

    auto camera_ut = lua.new_usertype<me::Camera>("Camera",
        sol::constructors<me::Camera()>(),
        "zoom",   &me::Camera::zoom,
        "smooth", &me::Camera::smooth
    );

    camera_ut.set_function("is_current",   &me::Camera::is_current);
    camera_ut.set_function("set_current",  &me::Camera::set_current);
    camera_ut.set_function("make_current", &me::Camera::make_current);
    camera_ut.set_function("get_current",  &me::Camera::get_current);

    auto tilemap_ut = lua.new_usertype<me::TileMap>("TileMap",
        sol::constructors<me::TileMap()>(),
        "origin_x", &me::TileMap::origin_x,
        "origin_y", &me::TileMap::origin_y,
        "width",    &me::TileMap::width,
        "height",   &me::TileMap::height,
        "dirty",    &me::TileMap::dirty,
        "tiles",    &me::TileMap::tiles,
        "tileset", sol::property(
            [](me::TileMap& tm) -> me::TileSet* {
                auto h = tm.tileset.handle();
                return h ? h.get() : nullptr;
            },
            [](me::TileMap& tm, me::TileSet& ts) {
                auto sp = me::ResourceLoader::find_handle(&ts);
                if (!sp) {
                    throw std::runtime_error(
                        "TileMap.tileset: TileSet not from ResourceLoader "
                        "(load it via ResourceLoader.load_tileset first)");
                }
                tm.tileset = entt::resource<me::TileSet>{sp};
                tm.dirty = true;
            }
        )
    );

    tilemap_ut.set_function("load_tiles", &me::TileMap::load_tiles);
    tilemap_ut.set_function("set_tile",   &me::TileMap::set_tile);
    tilemap_ut.set_function("get_tile",   &me::TileMap::get_tile);

    lua.new_usertype<me::Transform>("Transform",
        sol::constructors<me::Transform()>(),
        "position", &me::Transform::position,
        "rotation", &me::Transform::rotation,
        "scale",    &me::Transform::scale
    );

    lua.new_usertype<me::Velocity>("Velocity",
        sol::constructors<me::Velocity()>(),
        "linear",  &me::Velocity::linear,
        "angular", &me::Velocity::angular
    );

    lua.new_usertype<me::ui::FillRect>("FillRect",
        sol::constructors<me::ui::FillRect()>(),
        "foreground", &me::ui::FillRect::foreground,
        "dirty",      &me::ui::FillRect::dirty,
        "shape",      &me::ui::FillRect::shape,

        "stylebox", sol::property(
            [](me::ui::FillRect& fr) -> me::StyleBox* {
                auto h = fr.stylebox.handle();
                return h ? h.get() : nullptr;
            },
            [](me::ui::FillRect& fr, me::StyleBox& sb) {
                auto sp = me::ResourceLoader::find_handle(&sb);
                if (!sp) {
                    throw std::runtime_error(
                        "FillRect.stylebox: StyleBox not from ResourceLoader "
                        "(load it via ResourceLoader.load_stylebox first)"
                    );}
                
                fr.stylebox = entt::resource<me::StyleBox>{sp};
                fr.dirty = true;
            }
        )
    );

    lua.new_usertype<me::ui::Interactable>("Interactable",
        sol::constructors<me::ui::Interactable()>()
    );

    lua.new_usertype<me::ui::Label>("Label",
        "text", sol::property(
            [](me::ui::Label& l) -> std::string {
                return l.text->getString().toAnsiString();
            },
            [](me::ui::Label& l, const std::string& s) {
                l.text->setString(sf::String::fromUtf8(s.begin(), s.end()));
                l.dirty = true;
            }
        ),
        "font", sol::property(
            [](me::ui::Label& l) -> me::Font* {
                auto h = l.get_font().handle();
                return h ? h.get() : nullptr;
            },
            [](me::ui::Label& l, me::Font& f) {
                auto fh = me::ResourceLoader::find_handle(&f);
                if (!fh) {
                    throw std::runtime_error(
                        "Label.font not from ResourceLoader "
                        "(load it via ResourceLoader.load_Font first)"
                    );}
                
                l.set_font(f);
            }
        ),
        "character_size", sol::property(
            [](me::ui::Label& l) { return l.text->getCharacterSize(); },
            [](me::ui::Label& l, unsigned int s) { l.text->setCharacterSize(s); }
        ),
        "color", sol::property(
            [](me::ui::Label& l) { return l.text->getFillColor(); },
            [](me::ui::Label& l, sf::Color c) { l.text->setFillColor(c); }
        ),
        "position", sol::property(
            [](me::ui::Label& l) { return l.text->getPosition(); },
            [](me::ui::Label& l, sf::Vector2f p) { l.text->setPosition(p); }
        ),
        "dirty", &me::ui::Label::dirty
    );

    lua.new_usertype<me::ZIndex>("ZIndex",
        sol::constructors<me::ZIndex()>(),
        "value", &me::ZIndex::value
    );

    register_component<me::Transform>("Transform", lua);
    register_component<me::ZIndex>("ZIndex", lua);
    register_component<me::Velocity>("Velocity", lua);
    register_component<me::TileMap>("TileMap", lua);
    register_component<me::Camera>("Camera", lua);
    register_component<me::Sprite>("Sprite", lua);
    register_component<me::SpriteAnimation>("SpriteAnimation", lua);
    register_component<me::ui::FillRect>("FillRect", lua);
    register_component<me::ui::Label>("Label", lua);
    register_component<me::ui::Interactable>("Interactable", lua);
}

} // namespace me::lua_bindings