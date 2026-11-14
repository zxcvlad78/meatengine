#include "FactoryRuntime.hpp"

#include <meatengine/scripting/LuaBinder.hpp>

static void bind(sol::state& lua) {
    godlike::factory::bind_data_api(lua);
    godlike::factory::bind_control_api(lua);
    godlike::factory::bind_network();
}

ME_LUA_BINDING(
    "godlike:api:Factory",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)
