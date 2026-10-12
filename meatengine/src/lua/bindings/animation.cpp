#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::Animation::FrameData>("AnimationFrameData",
        sol::constructors<me::Animation::FrameData()>(),
        "x", &me::Animation::FrameData::x,
        "y", &me::Animation::FrameData::y,
        "w", &me::Animation::FrameData::w,
        "h", &me::Animation::FrameData::h
    );

    lua.new_usertype<me::Animation>("Animation",
        sol::constructors<me::Animation()>(),
        "name", &me::Animation::name,
        "fps", &me::Animation::fps,
        "is_looping", &me::Animation::is_looping,
        "frames", &me::Animation::frames,
        "duration", sol::property(
            [](me::Animation& a) { return a.duration(); }
        )
    );
}

ME_LUA_BINDING(
    "engine:Animation",
    static_cast<unsigned>(me::lua::BindPriority::Resource) - 1,
    &bind
)