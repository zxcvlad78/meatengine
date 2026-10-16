#include <string>

#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/meatnet/Network.hpp>
#include <meatengine/meatnet/Serialization.hpp>

static void bind(sol::state& lua) {

    sol::table table = lua.create_table();
    lua["Network"] = table;

    table.set_function("start_server", [](uint16_t port = me::net::kDefaultPort) {
        return me::net::Network::start_server(port);
    });

    table.set_function("connect", [](const std::string& address) {
        return me::net::Network::connect(address);
    });

    table.set_function("disconnect", []() {
        me::net::Network::disconnect();
    });

    table.set_function("is_server", []() {
        return me::net::Network::is_server();
    });

    table.set_function("is_client", []() {
        return me::net::Network::is_client();
    });

    table.set_function("chat_msg", [](const std::string& text) {
        if (auto* client = me::net::Network::client()) {
            me::net::BinaryWriter writer;
            writer.WriteUInt8(2);
            writer.WriteString(text);
            client->Send(writer.GetBuffer().data(), writer.Size(), true);
        } else if (auto* server = me::net::Network::server()) {
            std::string s = "Host: " + text;
            printf("[Network][Server][Chat] %s\n", text.c_str());
            me::net::BinaryWriter writer;
            writer.WriteUInt8(2);
            writer.WriteString(s);
            server->Send(writer.GetBuffer().data(), writer.Size(), true);
        }
    });
    
}

ME_LUA_BINDING(
    "engine:api:Network",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)