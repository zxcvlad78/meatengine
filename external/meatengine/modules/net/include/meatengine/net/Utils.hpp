#pragma once

#include "Types.hpp"

#include <functional>
#include <string>

namespace me::net {

enum class LogLevel {
    Debug = k_ESteamNetworkingSocketsDebugOutputType_Debug,
    Warning = k_ESteamNetworkingSocketsDebugOutputType_Warning,
    Error = k_ESteamNetworkingSocketsDebugOutputType_Error,
    Important = k_ESteamNetworkingSocketsDebugOutputType_Important,
    Msg = k_ESteamNetworkingSocketsDebugOutputType_Msg,
    Bug = k_ESteamNetworkingSocketsDebugOutputType_Bug,
};

using LogCallback = std::function<void(LogLevel, const char*)>;

bool InitNetwork();
void ShutdownNetwork();
void SetLogCallback(LogCallback cb);
void InternalDebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg);

void SetLogLevel(LogLevel level);
LogLevel GetLogLevel();

void SetSendRate(std::uint32_t bytes_per_sec);
std::uint32_t GetSendRate();

void ApplyConnectionTuning(ConnectionID conn);

ISteamNetworkingSockets* GetSocketsInterface();

}
