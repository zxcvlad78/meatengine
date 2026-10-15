#include <meatengine/meatnet/Network.hpp>

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

    printf("[Network] Server started on port '%u'\m", port);
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

    _client = client;
    printf("[Network] Connected to '%s'\n", address.c_str());
    return true;
}

}