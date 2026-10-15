#include <meatengine/meatnet/Network.hpp>

using namespace me::net;

void s_OnClientConnected(me::net::ConnectionID conn, const char* address) {
    printf("Client %s connected (ID: %u)\n", address, conn);
}

void s_OnClientDisconnected(ConnectionID conn, int reason, const char* debug) {
    printf("Client %u disconnected. Reason: %d (%s)\n", conn, reason, debug);
}

void s_OnMessageReceived(ConnectionID conn, const void* data, uint32_t size) {
    printf("Message Received from '%u'.\n", conn);
}

void c_OnConnected() {
    printf("Connected to server!\n");
}

void c_OnDisconnected(int reason, const char* debug) {
    printf("Disconnected. Reason: %d (%s)\n", reason, debug);
}

void c_OnMessageReceived(const void* data, uint32_t size) {
    printf("Message received");
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
    delete _client;
    _client = nullptr;

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

}