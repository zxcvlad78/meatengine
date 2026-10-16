#include <meatengine/meatnet/Network.hpp>
#include <meatengine/meatnet/Serialization.hpp>

#include <meatengine/MainLoop.hpp>
#include <meatengine/PackedEntity.hpp>
#include <meatengine/cmp/net/NetId.hpp>
#include <meatengine/cmp/net/Authority.hpp>
#include <meatengine/cmp/GlobalTransform.hpp>

using namespace me::net;

void s_OnClientConnected(ConnectionID conn, const char* address) {
    printf("[Network][Server] Client %s connected (ID: %u)\n", address, conn);

    auto* server_ = me::net::Network::server();
    if (!server_) return;

    BinaryWriter writer;
    writer.WriteUInt8(static_cast<uint8_t>(MsgType::SpawnPlayer));
    writer.WriteUInt32(conn);

    server_->Send(writer.GetBuffer().data(), writer.Size(), true);
}

void s_OnClientDisconnected(ConnectionID conn, int reason, const char* debug) {
    printf("[Network][Server] Client %u disconnected. Reason: %d (%s)\n", conn, reason, debug);
}

void s_OnMessageReceived(ConnectionID conn, const void* data, uint32_t size) {
    printf("[Network][Server] Message Received from '%u'. MsgType: ", conn);

    BinaryReader reader(static_cast<const uint8_t*>(data), size);
    uint8_t msg_type; if (!reader.ReadUInt8(msg_type)) {
        printf("Error Type\n");
        return;
    }
    printf("%u\n", msg_type);

    auto* server_ = me::net::Network::server();
    if (!server_) return;

    switch(msg_type) {
        case static_cast<uint8_t>(MsgType::ChatMessage): {
            std::string s; if (!reader.ReadString(s)) break;
            std::string text = std::to_string(conn) + ": " + s;
            printf("[Network][Server][Chat] %s\n", text.c_str());
            BinaryWriter writer;
            writer.WriteUInt8(static_cast<uint8_t>(MsgType::ChatMessage));
            writer.WriteString(text);
            server_->Send(writer.GetBuffer().data(), writer.Size(), true);
            break;
        }

        default: break;
    }
}

void c_OnConnected() {
    printf("[Network][Client] Connected to server!\n");
}

void c_OnDisconnected(int reason, const char* debug) {
    printf("[Network][Client] Disconnected. Reason: %d (%s)\n", reason, debug);
}

void c_OnMessageReceived(const void* data, uint32_t size) {
    printf("[Network][Client] Message received: ");

    BinaryReader reader(static_cast<const uint8_t*>(data), size);
    uint8_t msg_type; if (!reader.ReadUInt8(msg_type)) {
        printf("Error Type\n");
        return;
    }
    printf("%u\n", msg_type);

    auto* client_ = me::net::Network::client();
    if (!client_) return;

    switch(msg_type) {
        case static_cast<uint8_t>(MsgType::SpawnPlayer): {
            me::net::ConnectionID conn_id; if (!reader.ReadUInt32(conn_id)) break;
            auto& reg = me::MainLoop::get_registry();
            auto e = me::PackedEntity::spawn("core:player", reg);
            reg.emplace_or_replace<me::cmp::net::NetId>(e);
            auto& auth = reg.emplace_or_replace<me::cmp::net::Authority>(e);
            auth.id = conn_id;


            break;
        }
        case static_cast<uint8_t>(MsgType::SyncTransform): {
            uint32_t net_id = 0;
            if (!reader.ReadUInt32(net_id)) break;

            auto& r = me::MainLoop::get_registry();
            auto e = me::net::Network::resolve(net_id);
            if (e == entt::null) break;

            me::cmp::GlobalTransform gt;
            if (!me::cmp::deserialize(reader, gt)) break;

            r.emplace_or_replace<me::cmp::GlobalTransform>(e, gt);

            break;
        }
        case static_cast<uint8_t>(MsgType::ChatMessage): {
            std::string s; if (!reader.ReadString(s)) break;
            printf("'[Network][Client][Chat] %s'\n", s.c_str());
            break;
        }

        default: break;
    }
}

namespace me::net {

Server* Network::server() {
    return _server;
}

Client* Network::client() {
    return _client;
}


bool Network::start_server(uint16_t port) {
    if (_server != nullptr) return false;
    delete _client;
    _client = nullptr;

    _server = new Server();
    if (!_server->Start(port)) {
        fprintf(stderr, "[Network] Failed to start server\n");
        delete _server;
        ShutdownNetwork();
        return false;
    }

    _server->SetOnClientConnected(s_OnClientConnected);
    _server->SetOnClientDisconnected(s_OnClientDisconnected);
    _server->SetOnMessageReceived(s_OnMessageReceived);

    printf("[Network] Server started on port '%u'\n", port);
    return true;
}


bool Network::connect(const std::string& address) {
    disconnect();

    auto* client = new Client();
    if (!client->Connect(address)) {
        fprintf(stderr, "[Network] Failed to connect\n");
        delete client;
        return false;
    }

    client->SetOnConnected(c_OnConnected);
    client->SetOnDisconnected(c_OnDisconnected);
    client->SetOnMessageReceived(c_OnMessageReceived);
    _client = client;
    printf("[Network] Connected to '%s'\n", address.c_str());
    return true;
}

void Network::disconnect() {
    delete _server;
    _server = nullptr;
    delete _client;
    _client = nullptr;
}

void Network::assign(uint64_t id, entt::entity e) {
    _nid_map[id] = e;
    _entity_map[e] = id;
}

entt::entity Network::resolve(uint64_t id) {
    auto it = _nid_map.find(id);
    return it != _nid_map.end() ? it->second : entt::null;
}

uint64_t Network::resolve(entt::entity e) {
    auto it = _entity_map.find(e);
    return it != _entity_map.end() ? it->second : entt::null;
}

void Network::remove(std::uint64_t id) {
    auto it = _nid_map.find(id);
    if (it == _nid_map.end()) return;
    _nid_map.erase(it);
    _entity_map.erase(it->second);
}

void Network::remove(entt::entity e) {
    auto it = _entity_map.find(e);
    if (it == _entity_map.end()) return;
    _nid_map.erase(it->second);
    _entity_map.erase(it);
}

bool Network::is_authority(entt::entity e) {
    auto& reg = MainLoop::get_registry();
	me::net::ConnectionID conn_id;
	if (auto* c = me::net::Network::client()) conn_id = c->GetConnectionID();
	else conn_id = 0;
    
    uint32_t authority;
    if (auto* auth = reg.try_get<me::cmp::net::Authority>(e)) authority = auth->id;
    else authority = 0;

    return conn_id == authority;
}

bool Network::is_server() {
    return _server != nullptr;
}

bool Network::is_client() {
    return _client != nullptr;
}

}