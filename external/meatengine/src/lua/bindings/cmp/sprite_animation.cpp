#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/SpriteAnimation.hpp>

static void bind(sol::state& lua) {
    auto usertype = lua.new_usertype<me::cmp::SpriteAnimation>("SpriteAnimation",
        sol::constructors<me::cmp::SpriteAnimation()>(),

        "spritesheet", sol::property(
            [](me::cmp::SpriteAnimation& sa) -> me::SpriteSheet* {
                return sa.spritesheet ? sa.spritesheet.operator->() : nullptr;
            },
            [](me::cmp::SpriteAnimation& sa, me::SpriteSheet& sheet) {
                auto h = me::ResourceLoader::find_handle(&sheet);
                if (!h) {
                    throw std::runtime_error(
                        "SpriteAnimation.spritesheet: SpriteSheet not from ResourceLoader "
                        "(load it via ResourceLoader.load_spritesheet first)"
                    );
                }
                sa.spritesheet = entt::resource<me::SpriteSheet>{h};
            }
        ),

        "is_playing", &me::cmp::SpriteAnimation::is_playing,
        "current_frame_idx", &me::cmp::SpriteAnimation::current_frame_idx,
        "time_accumulator", &me::cmp::SpriteAnimation::time_accumulator,
        "next_anim", &me::cmp::SpriteAnimation::next_anim,

        "play", [](me::cmp::SpriteAnimation& sa,
                const std::string& name,
                sol::optional<std::string> next) {
            return sa.play(name, next.value_or(""));
        }
    );

    me::lua::register_component<me::cmp::SpriteAnimation>("SpriteAnimation", lua);
}

ME_LUA_BINDING(
    "engine:cmp:SpriteAnimation",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)