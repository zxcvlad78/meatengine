#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <meatengine/net/Serialization.hpp>
#include <meatengine/net/Protocol.hpp>
#include <meatengine/net/Network.hpp>

namespace me::net {

struct RpcContext {
    PeerId sender = kInvalidPeerId;
    bool from_server = false;
    std::uint32_t request_id = 0;

    std::vector<std::uint8_t> reply_payload;
    bool has_reply = false;

    template<typename... Args>
    void reply(Args&&... args) {
        BinaryWriter w;
        (w.write(std::forward<Args>(args)), ...);
        reply_payload = std::move(w).buffer();
        has_reply = true;
    }

    void reply_empty() {
        reply_payload.clear();
        has_reply = true;
    }
};

using RpcHandler = std::function<void(RpcContext&, BinaryReader&)>;

struct PendingRpc {
    std::function<void(BinaryReader&)> on_reply;
    std::function<void()> on_timeout;
    float deadline = 0.f;
};

class Rpc {
public:
    Rpc() = delete;

    static void bind(const std::string& name, RpcHandler h);
    static bool has(const std::string& name);

    template<typename... Args>
    static void invoke(const std::string& name, Args&&... args) {
        send_call(kServerPeerId, 0u, name, [&](BinaryWriter& w) {
            (w.write(std::forward<Args>(args)), ...);
        });
    }

    template<typename... Args>
    static void invoke(PeerId target, const std::string& name, Args&&... args) {
        send_call(target, 0u, name, [&](BinaryWriter& w) {
            (w.write(std::forward<Args>(args)), ...);
        });
    }

    template<typename F>
    static void broadcast(const std::string& name, F&& write_args) {
        BinaryWriter args_w;
        write_args(args_w);
        auto args_payload = std::move(args_w).buffer();

        if (Network::is_server())
            run_local_handler(name, args_payload);

        Network::send(MsgType::RpcCall, SendFlag::Reliable,
            [&name, &args_payload](BinaryWriter& w) {
                w.write(static_cast<std::uint32_t>(0));
                w.write(name);
                if (!args_payload.empty())
                    w.write(args_payload.data(), args_payload.size());
            });
    }

    static void broadcast(const std::string& name, const std::vector<std::uint8_t>& args) {
        broadcast(name, [&args](BinaryWriter& w) {
            if (!args.empty()) w.write(args.data(), args.size());
        });
    }

    static std::uint32_t invoke_async(
        const std::string& name,
        std::function<void(BinaryReader&)> on_reply,
        std::function<void()> on_timeout,
        std::vector<std::uint8_t> args,
        float timeout_sec = 5.f);

    static std::uint32_t invoke_async(
        PeerId target,
        const std::string& name,
        std::function<void(BinaryReader&)> on_reply,
        std::function<void()> on_timeout,
        std::vector<std::uint8_t> args,
        float timeout_sec = 5.f);

    static void handle_call(PeerId sender, bool on_server, BinaryReader& r);
    static void handle_reply(BinaryReader& r);

    static void update(float dt);
    static void clear();

private:
    template<typename F>
    static void send_call(PeerId target, std::uint32_t rid, const std::string& name, F&& write) {
        Network::send(target, MsgType::RpcCall, SendFlag::Reliable,
            [&](BinaryWriter& w) {
                w.write(rid);
                w.write(name);
                write(w);
            });
    }

    static void run_local_handler(const std::string& name, const std::vector<std::uint8_t>& args);

    static std::unordered_map<std::string, RpcHandler>& handlers();
    static std::unordered_map<std::uint32_t, PendingRpc>& pending();

    static std::uint32_t next_request_id();
    inline static std::uint32_t _next_id = 1;
};

}