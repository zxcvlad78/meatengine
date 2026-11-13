#pragma once

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace me::net {

using ConnectionID = HSteamNetConnection;
using ListenSocket = HSteamListenSocket;
using PollGroup = HSteamNetPollGroup;

using PeerId = std::uint32_t;

constexpr PeerId kInvalidPeerId = 0xFFFFFFFFu;
constexpr PeerId kServerPeerId = 0u;

constexpr std::uint16_t kDefaultPort = 2302;

constexpr std::size_t kMaxMessageSize = 512 * 1024;

constexpr int kMaxMessagesPerPoll = 256;




using OnClientConnectedCallback = std::function<void(ConnectionID, const char*)>;





using OnClientDisconnectedCallback = std::function<void(ConnectionID, int, const char*)>;





using OnMessageReceivedCallback = std::function<void(PeerId, ConnectionID, const void*, uint32_t)>;


using OnConnectedCallback = std::function<void()>;




using OnDisconnectedCallback = std::function<void(int, const char*)>;


using OnServerMessageCallback = std::function<void(const void*, uint32_t)>;

struct ClientInfo {
    ConnectionID id = k_HSteamNetConnection_Invalid;
    std::string address;

};

}
