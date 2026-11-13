#include <meatengine/net/Utils.hpp>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

namespace me::net {

static LogCallback g_logCallback = nullptr;
static bool g_bInitialized = false;
static ISteamNetworkingSockets* g_pSockets = nullptr;
static LogLevel g_logLevel = LogLevel::Warning;
static std::uint32_t g_sendRate = 64 * 1024 * 1024;

static void NukeProcess(int rc) {
#ifdef _WIN32
    ExitProcess(rc);
#else
    (void)rc;
    kill(getpid(), SIGKILL);
#endif
}

void InternalDebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg) {
    LogLevel level = static_cast<LogLevel>(eType);
    if (g_logCallback) {
        g_logCallback(level, pszMsg);
    } else {
        printf("[%s] %s\n",
               eType == k_ESteamNetworkingSocketsDebugOutputType_Bug ? "FATAL" :
               eType == k_ESteamNetworkingSocketsDebugOutputType_Error ? "ERROR" :
               eType == k_ESteamNetworkingSocketsDebugOutputType_Warning ? "WARN" :
               eType == k_ESteamNetworkingSocketsDebugOutputType_Msg ? "INFO" : "DEBUG",
               pszMsg);
        fflush(stdout);
    }

    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
        fflush(stderr);
        NukeProcess(1);
    }
}

bool InitNetwork() {
    if (g_bInitialized)
        return true;

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
        fprintf(stderr, "GameNetworkingSockets_Init failed: %s\n", errMsg);
        return false;
    }
#else
    SteamDatagram_SetAppID(0xFF);
    SteamDatagram_SetUniverse(false, k_EUniverseDev);

    SteamDatagramErrMsg errMsg;
    if (!SteamDatagramClient_Init(errMsg)) {
        fprintf(stderr, "SteamDatagramClient_Init failed: %s\n", errMsg);
        return false;
    }

    SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);

    SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_RecvBufferSize, 16 * 1024 * 1024);
    SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_RecvBufferMessages, 8192);
#endif

    SteamNetworkingUtils()->SetDebugOutputFunction(
        static_cast<ESteamNetworkingSocketsDebugOutputType>(g_logLevel), InternalDebugOutput);

    g_pSockets = SteamNetworkingSockets();
    g_bInitialized = true;
    return true;
}

void ShutdownNetwork() {
    if (!g_bInitialized)
        return;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
    GameNetworkingSockets_Kill();
#else
    SteamDatagramClient_Kill();
#endif

    g_pSockets = nullptr;
    g_bInitialized = false;
}

void SetLogCallback(LogCallback cb) {
    g_logCallback = cb;
}

void SetLogLevel(LogLevel level) {
    g_logLevel = level;
    if (g_bInitialized)
        SteamNetworkingUtils()->SetDebugOutputFunction(
            static_cast<ESteamNetworkingSocketsDebugOutputType>(g_logLevel), InternalDebugOutput);
}

LogLevel GetLogLevel() {
    return g_logLevel;
}

void SetSendRate(std::uint32_t bytes_per_sec) {
    g_sendRate = bytes_per_sec;
}

std::uint32_t GetSendRate() {
    return g_sendRate;
}

void ApplyConnectionTuning(ConnectionID conn) {
    auto* utils = SteamNetworkingUtils();
    if (!utils) return;

    utils->SetConnectionConfigValueInt32(conn, k_ESteamNetworkingConfig_SendBufferSize, 16 * 1024 * 1024);

    if (g_sendRate > 0) {
        utils->SetConnectionConfigValueInt32(conn, k_ESteamNetworkingConfig_SendRateMin,
                                             static_cast<int32_t>(g_sendRate));
        utils->SetConnectionConfigValueInt32(conn, k_ESteamNetworkingConfig_SendRateMax,
                                             static_cast<int32_t>(g_sendRate));
    }

    utils->SetConnectionConfigValueInt32(conn, k_ESteamNetworkingConfig_NagleTime, 0);
}

ISteamNetworkingSockets* GetSocketsInterface() {
    return g_pSockets;
}

}
