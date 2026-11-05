#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/Sprite.hpp>

static void bind(sol::state& lua) {
    auto sprite_ut = lua.new_usertype<me::cmp::Sprite>("Sprite",
        sol::constructors<me::cmp::Sprite()>(),

        "texture", sol::property(
            [](me::cmp::Sprite& sp) -> me::Texture* { return sp.texture_ptr(); },
            [](me::cmp::Sprite& sp, me::Texture& t) { sp.set_texture(t); }
        ),

        "offset", &me::cmp::Sprite::offset,
        "center", &me::cmp::Sprite::center,

        "has_texture", &me::cmp::Sprite::has_texture,
        "has_sprite",  &me::cmp::Sprite::has_sprite,
        "clear_texture", &me::cmp::Sprite::clear_texture,

        "sf_sprite", sol::property(
            [](me::cmp::Sprite& sp) -> sf::Sprite* { return sp.sprite_ptr(); },
            [](me::cmp::Sprite&, sol::object) {
                throw std::runtime_error("sf_sprite is read-only");
            }
        )
    );
    
    me::lua::register_component<me::cmp::Sprite>("Sprite", lua);
}

ME_LUA_BINDING(
    "engine:cmp:Sprite",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)