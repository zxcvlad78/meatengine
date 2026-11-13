#include <meatengine/net/Network.hpp>

#include <chrono>

namespace me::net {

std::uint32_t Network::now_ms() {
    static const auto t0 = std::chrono::steady_clock::now();
    return static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count());
}

NetworkStats& Network::stats() { return _stats; }

void Network::reset_stats() {
    _stats.reset();
    _stats_window_ms = now_ms();
    _stats_win_messages = 0;
    _stats_win_bytes = 0;
    _stats_win_rpcs = 0;
}

void Network::update_stats() {
    const std::uint32_t now = now_ms();
    if (_stats_window_ms == 0) {
        _stats_window_ms = now;
        return;
    }

    const std::uint32_t dt = now - _stats_window_ms;
    if (dt < 500) return;

    const double secs = static_cast<double>(dt) * 0.001;
    _stats.messages_per_sec = static_cast<double>(_stats.messages_sent - _stats_win_messages) / secs;
    _stats.rpcs_per_sec = static_cast<double>(_stats.rpcs_sent - _stats_win_rpcs) / secs;
    _stats.megabytes_per_sec =
        static_cast<double>(_stats.bytes_sent - _stats_win_bytes) / secs / (1024.0 * 1024.0);

    _stats_window_ms = now;
    _stats_win_messages = _stats.messages_sent;
    _stats_win_bytes = _stats.bytes_sent;
    _stats_win_rpcs = _stats.rpcs_sent;
}

}
