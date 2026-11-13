#include <meatengine/net/Server.hpp>

#include <algorithm>
#include <cassert>
#include <cstdio>

namespace me::net {

Server* Server::s_pCallbackInstance = nullptr;

Server::Server()
    : NetworkPeer()
    , m_hListenSocket(k_HSteamListenSocket_Invalid)
    , m_hPollGroup(k_HSteamNetPollGroup_Invalid)
    , m_running(false)
{
}

Server::~Server() {
    Close();
}

bool Server::Start(uint16_t port) {
    if (m_running) {
        Log(LogLevel::Warning, "Server already running");
        return false;
    }
    if (!m_pInterface) {
        Log(LogLevel::Error, "SteamNetworkingSockets interface not available");
        return false;
    }

    SteamNetworkingIPAddr localAddr;
    localAddr.Clear();
    localAddr.m_port = port;

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);

    s_pCallbackInstance = this;

    m_hListenSocket = m_pInterface->CreateListenSocketIP(localAddr, 1, &opt);
    if (m_hListenSocket == k_HSteamListenSocket_Invalid) {
        s_pCallbackInstance = nullptr;
        Log(LogLevel::Error, "Failed to create listen socket");
        return false;
    }

    m_hPollGroup = m_pInterface->CreatePollGroup();
    if (m_hPollGroup == k_HSteamNetPollGroup_Invalid) {
        Log(LogLevel::Error, "Failed to create poll group");
        m_pInterface->CloseListenSocket(m_hListenSocket);
        m_hListenSocket = k_HSteamListenSocket_Invalid;
        s_pCallbackInstance = nullptr;
        return false;
    }

    m_running = true;
    Log(LogLevel::Msg, ("Server started on port " + std::to_string(port)).c_str());
    return true;
}

void Server::Close() {
    if (!m_running) return;
    m_running = false;

    {
        std::lock_guard<std::mutex> lock(m_clientMutex);
        for (auto& pair : m_clients) {
            m_pInterface->CloseConnection(pair.first, 0, "Server shutdown", true);
        }
        m_clients.clear();
        m_conn_list.clear();
        m_peer_by_conn.clear();
    }

    if (m_hListenSocket != k_HSteamListenSocket_Invalid) {
        m_pInterface->CloseListenSocket(m_hListenSocket);
        m_hListenSocket = k_HSteamListenSocket_Invalid;
    }
    if (m_hPollGroup != k_HSteamNetPollGroup_Invalid) {
        m_pInterface->DestroyPollGroup(m_hPollGroup);
        m_hPollGroup = k_HSteamNetPollGroup_Invalid;
    }
    s_pCallbackInstance = nullptr;
    Log(LogLevel::Msg, "Server stopped");
}

void Server::Update() {
    if (!m_running) return;
    PollCallbacks();
    PollIncomingMessages();
}

bool Server::Send(const void* data, uint32_t size, bool reliable) {
    Broadcast(data, size, k_HSteamNetConnection_Invalid,
              reliable ? SendFlag::Reliable : SendFlag::Unreliable);
    return true;
}

bool Server::Send(const void* data, uint32_t size, SendFlag flag) {
    Broadcast(data, size, k_HSteamNetConnection_Invalid, flag);
    return true;
}

bool Server::SendToClient(ConnectionID client, const void* data, uint32_t size, bool reliable) {
    return SendToClient(client, data, size, reliable ? SendFlag::Reliable : SendFlag::Unreliable);
}

bool Server::SendToClient(ConnectionID client, const void* data, uint32_t size, SendFlag flag) {
    if (!m_running || !m_pInterface) return false;
    const int flags = gns_flag(flag);
    return m_pInterface->SendMessageToConnection(client, data, size, flags, nullptr) == k_EResultOK;
}

void Server::Broadcast(const void* data, uint32_t size, ConnectionID exclude, bool reliable) {
    Broadcast(data, size, exclude, reliable ? SendFlag::Reliable : SendFlag::Unreliable);
}

void Server::Broadcast(const void* data, uint32_t size, ConnectionID exclude, SendFlag flag) {
    static thread_local std::vector<ConnectionID> scratch;
    {
        std::lock_guard<std::mutex> lock(m_clientMutex);
        scratch.clear();
        scratch.reserve(m_conn_list.size());
        for (auto id : m_conn_list)
            if (id != exclude) scratch.push_back(id);
    }

    for (auto id : scratch)
        SendToClient(id, data, size, flag);
}

void Server::Kick(ConnectionID client, const char* reason) {
    if (m_pInterface) {
        m_pInterface->CloseConnection(client, 0, reason ? reason : "Kicked", true);
    }
}

std::vector<ConnectionID> Server::GetConnectedClients() const {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    return m_conn_list;
}

size_t Server::GetClientCount() const {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    return m_clients.size();
}

bool Server::GetClientInfo(ConnectionID client, ClientInfo& outInfo) const {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    auto it = m_clients.find(client);
    if (it == m_clients.end()) return false;
    outInfo = it->second;
    return true;
}

void Server::AssignPeer(ConnectionID conn, PeerId id) {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    m_peer_by_conn[conn] = id;
}

void Server::UnassignPeer(ConnectionID conn) {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    m_peer_by_conn.erase(conn);
}

PeerId Server::PeerOf(ConnectionID conn) const {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    auto it = m_peer_by_conn.find(conn);
    return it == m_peer_by_conn.end() ? kInvalidPeerId : it->second;
}

void Server::OnConnectionStateChange(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    ConnectionID conn = pInfo->m_hConn;

    switch (pInfo->m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
            bool was_known = false;
            {
                std::lock_guard<std::mutex> lock(m_clientMutex);
                auto it = m_clients.find(conn);
                if (it != m_clients.end()) {
                    m_clients.erase(it);
                    was_known = true;
                }
                m_conn_list.erase(
                    std::remove(m_conn_list.begin(), m_conn_list.end(), conn),
                    m_conn_list.end()
                );
            }

            if (was_known && m_onDisconnected) {
                m_onDisconnected(conn, pInfo->m_info.m_eEndReason, pInfo->m_info.m_szEndDebug);
            }
            {
                std::lock_guard<std::mutex> lock(m_clientMutex);
                m_peer_by_conn.erase(conn);
            }

            m_pInterface->CloseConnection(conn, 0, nullptr, false);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting: {
            if (m_pInterface->AcceptConnection(conn) != k_EResultOK) {
                m_pInterface->CloseConnection(conn, 0, nullptr, false);
                break;
            }
            if (!m_pInterface->SetConnectionPollGroup(conn, m_hPollGroup)) {
                m_pInterface->CloseConnection(conn, 0, nullptr, false);
                break;
            }
            ClientInfo info;
            info.id = conn;
            info.address = pInfo->m_info.m_szConnectionDescription;
            {
                std::lock_guard<std::mutex> lock(m_clientMutex);
                m_clients[conn] = info;
                m_conn_list.push_back(conn);
            }
            if (m_onConnected) {
                m_onConnected(conn, info.address.c_str());
            }
            break;
        }

        case k_ESteamNetworkingConnectionState_Connected: {
            ApplyConnectionTuning(conn);
            break;
        }

        default:
            break;
    }
}

void Server::OnIncomingMessage(ConnectionID conn, const void* data, uint32_t size) {
    if (m_onMessage) {
        m_onMessage(PeerOf(conn), conn, data, size);
    }
}

void Server::PollIncomingMessages() {
    ISteamNetworkingMessage* messages[kMaxMessagesPerPoll];
    int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, messages, kMaxMessagesPerPoll);
    while (numMsgs > 0) {
        if (numMsgs < 0) {
            Log(LogLevel::Error, "ReceiveMessagesOnPollGroup error");
            break;
        }
        for (int i = 0; i < numMsgs; ++i) {
            OnIncomingMessage(messages[i]->m_conn, messages[i]->m_pData, messages[i]->m_cbSize);
            messages[i]->Release();
        }
        numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, messages, kMaxMessagesPerPoll);
    }
}

void Server::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    if (s_pCallbackInstance) {
        s_pCallbackInstance->OnConnectionStateChange(pInfo);
    }
}

}
