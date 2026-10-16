#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include <entt/entity/entity.hpp>

#include <meatengine/meatnet/Client.hpp>
#include <meatengine/meatnet/Server.hpp>


namespace me::net {

enum class MsgType : uint8_t {
    SpawnPlayer = 0,
    SyncTransform,
    ChatMessage
};



// Network (Manager xD)
class Network {
public:
    Network() = delete;

    static Server* server();
    static Client* client();

    static bool start_server(uint16_t port = kDefaultPort);
    static bool connect(const std::string& address);
    static void disconnect();

    static void assign(uint64_t id, entt::entity e);

    static entt::entity resolve(uint64_t id);
    static uint64_t resolve(entt::entity e);

    static void remove(uint64_t id);
    static void remove(entt::entity e);

    static bool is_server();
    static bool is_client();
    static bool is_authority(entt::entity e);

private:
    inline static std::unordered_map<uint64_t, entt::entity> _nid_map;
    inline static std::unordered_map<entt::entity, uint64_t> _entity_map;

    inline static Server* _server = nullptr;
    inline static Client* _client = nullptr;
};

}