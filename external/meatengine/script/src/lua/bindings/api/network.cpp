#include <string>

#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/net/Network.hpp>
#include <meatengine/net/Serialization.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::net::PeerInfo>("PeerInfo",
        sol::no_constructor,
        "connection_id", &me::net::PeerInfo::conn,
        "address", &me::net::PeerInfo::address,
        "latency", &me::net::PeerInfo::latency_ms
    );

    sol::table table = lua.create_table();
    lua["Network"] = table;

    table.set_function("start_server", [](sol::optional<uint16_t> port) {
        return me::net::Network::start_server(port.value_or(me::net::kDefaultPort));
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

    table.set_function("peer_id",
        []() { return me::net::Network::peer_id(); });

    table.set_function("peers",
        []() { return me::net::Network::peers(); });

    table.set_function("spawn_entity",
        [](const std::string& id, uint32_t owner) {
            return me::net::Network::spawn_entity(id, owner);
        });

    table.set_function("despawn_entity",
        [](entt::entity e) { me::net::Network::despawn_entity(e); });

    table.set_function("set_authority",
        [](entt::entity e, uint32_t owner) {
            me::net::Network::set_authority(e, owner);
        });

    table.set_function("set_on_peer_connected",
        [](sol::protected_function fn) {
            me::net::Network::set_on_peer_connected(
                [fn](me::net::PeerId pid) {
                    auto r = fn(pid);
                    if (!r.valid()) {
                        sol::error e = r;
                        std::cerr << "[Network.on_peer_joined] " << e.what() << "\n";
                    }
                });
        });

    table.set_function("set_on_peer_disconnected",
        [](sol::protected_function fn) {
            me::net::Network::set_on_peer_disconnected(
                [fn](me::net::PeerId pid) {
                    auto r = fn(pid);
                    if (!r.valid()) {
                        sol::error e = r;
                        std::cerr << "[Network.on_peer_left] " << e.what() << "\n";
                    }
                });
        });

    table.set_function("set_auto_cleanup_owned",
        [](bool v) { me::net::Network::set_auto_cleanup_owned(v); });

    table.set_function("get_auto_cleanup_owned",
        []() { return me::net::Network::get_auto_cleanup_owned(); });
    
}

ME_LUA_BINDING(
    "engine:api:Network",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)