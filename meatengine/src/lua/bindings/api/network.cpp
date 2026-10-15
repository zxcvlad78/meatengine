#include <string>

#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/meatnet/Network.hpp>

static void bind(sol::state& lua) {

    sol::table table = lua.create_table();
    lua["Network"] = table;

    table.set_function("start_server", [](uint16_t port = me::net::kDefaultPort) {
        return me::net::Network::start_server(port);
    });

    table.set_function("connect", [](const std::string& address) {
        return me::net::Network::connect(address);
    });
    
}

ME_LUA_BINDING(
    "engine:api:Network",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)