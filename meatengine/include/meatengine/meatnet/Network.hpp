#pragma once

#include <string>
#include <vector>

#include <meatengine/meatnet/Client.hpp>
#include <meatengine/meatnet/Server.hpp>

namespace me::net {

class Network {
public:
    Network() = delete;

    static Server* server();
    static Client* client();

    static bool start_server(uint16_t port = kDefaultPort);
    static bool connect(const std::string& address);

private:
    inline static Server* _server = nullptr;
    inline static Client* _client = nullptr;
};

}