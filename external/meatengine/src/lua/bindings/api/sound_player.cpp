#include <string>
#include <stdexcept>

#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/SoundPlayer.hpp>
#include <meatengine/Resources.hpp>
#include <meatengine/ResourceLoader.hpp>

namespace {
entt::resource<me::SoundBuffer> resolve_sound_buffer(me::SoundBuffer& sb) {
    auto h = me::ResourceLoader::find_handle(&sb);
    if (!h) {
        throw std::runtime_error(
            "SoundPlayer.play: SoundBuffer not from ResourceLoader "
            "(load it via ResourceLoader.load_SoundBuffer first)");
    }
    return entt::resource<me::SoundBuffer>{h};
}
}

static void bind(sol::state& lua) {
    sol::table table = lua.create_table();
    lua["SoundPlayer"] = table;

    table.set_function("play", [](me::SoundBuffer& sb,
           sol::optional<sf::Vector2f> position,
           sol::optional<float> min_distance,
           sol::optional<int> priority,
           sol::optional<float> attenuation
        ) {
            me::SoundPlayer::play(
                resolve_sound_buffer(sb),
                position.value_or(sf::Vector2f{0.f, 0.f}),
                min_distance.value_or(100.f),
                priority.value_or(0),
                attenuation.value_or(1.f)
            );
        }
    );

    table.set_function("stop_all",
        []() { me::SoundPlayer::stop_all(); });
}

ME_LUA_BINDING(
    "engine:api:SoundPlayer",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)