#include <godlike/lua_bindings/common.hpp>
#include <godlike/components/Common.hpp>

namespace godlike::lua_bindings {
    void init(sol::state& lua) {
        auto move_speed_ut = lua.new_usertype<components::MoveSpeed>("MoveSpeed",
            sol::constructors<components::MoveSpeed()>(),
            "value", &components::MoveSpeed::value
        );

        me::lua_bindings::register_component<components::MoveSpeed>("MoveSpeed", lua);
    }
}