#include <string>
#include <meatengine/scripting/LuaBinder.hpp>

#include <meatengine/graphics/cmp/Sprite.hpp>

static void bind(sol::state& lua) {
    auto sprite_ut = lua.new_usertype<me::graphics::cmp::Sprite>("Sprite",
        sol::constructors<me::graphics::cmp::Sprite()>(),

        "texture", sol::property(
            [](me::graphics::cmp::Sprite& sp) -> me::Texture* { return sp.texture_ptr(); },
            [](me::graphics::cmp::Sprite& sp, me::Texture& t) { sp.set_texture(t); }
        ),

        "offset", &me::graphics::cmp::Sprite::offset,
        "center", &me::graphics::cmp::Sprite::center,

        "has_texture", &me::graphics::cmp::Sprite::has_texture,
        "has_sprite",  &me::graphics::cmp::Sprite::has_sprite,
        "clear_texture", &me::graphics::cmp::Sprite::clear_texture,

        "sf_sprite", sol::property(
            [](me::graphics::cmp::Sprite& sp) -> sf::Sprite* { return sp.sprite_ptr(); },
            [](me::graphics::cmp::Sprite&, sol::object) {
                throw std::runtime_error("sf_sprite is read-only");
            }
        )
    );
    
    me::lua::register_component<me::graphics::cmp::Sprite>("Sprite", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Sprite",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)