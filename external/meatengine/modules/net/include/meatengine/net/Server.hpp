#pragma once

#include "NetworkPeer.hpp"

#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <meatengine/net/Protocol.hpp>

namespace me::net {

class Server : public NetworkPeer {
public:
    Server();
    ~Server() override;

    bool Start(uint16_t port = kDefaultPort);
    void Close() override;

    void Update() override;
    bool Send(const void* data, uint32_t size, bool reliable = true) override;
    bool Send(const void* data, uint32_t size, SendFlag flag);
    bool IsActive() const override { return m_running; }

    bool SendToClient(ConnectionID client, const void* data, uint32_t size, bool reliable = true);
    bool SendToClient(ConnectionID client, const void* data, uint32_t size, SendFlag flag);

    void Broadcast(const void* data, uint32_t size, ConnectionID exclude = k_HSteamNetConnection_Invalid, bool reliable = true);
    void Broadcast(const void* data, uint32_t size, ConnectionID exclude, SendFlag flag);

    void Kick(ConnectionID client, const char* reason = nullptr);
    std::vector<ConnectionID> GetConnectedClients() const;
    size_t GetClientCount() const;
    bool GetClientInfo(ConnectionID client, ClientInfo& outInfo) const;

    void AssignPeer(ConnectionID conn, PeerId id);
    void UnassignPeer(ConnectionID conn);
    PeerId PeerOf(ConnectionID conn) const;

    void SetOnClientConnected(OnClientConnectedCallback cb)    { m_onConnected = cb; }
    void SetOnClientDisconnected(OnClientDisconnectedCallback cb) { m_onDisconnected = cb; }
    void SetOnMessageReceived(OnMessageReceivedCallback cb)   { m_onMessage = cb; }

protected:
    void OnIncomingMessage(ConnectionID conn, const void* data, uint32_t size) override;
    void OnConnectionStateChange(SteamNetConnectionStatusChangedCallback_t* pInfo) override;

private:
    ListenSocket m_hListenSocket;
    PollGroup m_hPollGroup;
    std::map<ConnectionID, ClientInfo> m_clients;
    std::vector<ConnectionID> m_conn_list;
    std::unordered_map<ConnectionID, PeerId> m_peer_by_conn;
    mutable std::mutex m_clientMutex;
    bool m_running;

    OnClientConnectedCallback m_onConnected;
    OnClientDisconnectedCallback m_onDisconnected;
    OnMessageReceivedCallback m_onMessage;

    void PollIncomingMessages();

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
    static Server* s_pCallbackInstance;
};

}
