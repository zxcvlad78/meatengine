#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <meatengine/net/Serialization.hpp>
#include <meatengine/net/Protocol.hpp>

namespace me::net {

namespace rpc {
    namespace detail {
        template<typename T>
        using bare_t = std::remove_cv_t<std::remove_reference_t<T>>;

        template<typename T>
        inline constexpr bool is_packable_v =
            std::is_arithmetic_v<bare_t<T>> ||
            std::is_same_v<bare_t<T>, std::string> ||
            std::is_same_v<std::decay_t<T>, std::vector<std::uint8_t>> ||
            std::is_convertible_v<std::decay_t<T>, const char*>;

        template<typename... Args>
        inline constexpr bool all_packable_v = (is_packable_v<Args> && ...);
    }

    template<typename... Args>
    std::vector<std::uint8_t> pack(Args&&... args) {
        static_assert(
            detail::all_packable_v<Args...>,
            "rpc::pack(): supported types are arithmetic, std::string, "
            "const char*, and std::vector<std::uint8_t>"
        );

        BinaryWriter w;
        (w.write(std::forward<Args>(args)), ...);
        return w.buffer();
    }
}

struct RpcContext {
    PeerId sender = kInvalidPeerId;
    bool from_server = false;
    std::uint32_t request_id = 0;

    std::vector<std::uint8_t> reply_payload;
    bool has_reply = false;

    template<typename... Args>
    void reply(Args&&... args) {
        reply_payload = rpc::pack(std::forward<Args>(args)...);
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
    inline static void invoke(const std::string& name, Args&&... args) {
        send(0u, name, rpc::pack(std::forward<Args>(args)...));
    }

    template<typename... Args>
    inline static void invoke(PeerId target, const std::string& name, Args&&... args) {
        send(target, 0u, name, rpc::pack(std::forward<Args>(args)...));
    }

    static void broadcast(const std::string& name, const std::vector<std::uint8_t>& args);

    static std::uint32_t invoke_async(
        const std::string& name,
        std::function<void(BinaryReader&)> on_reply,
        std::function<void()> on_timeout,
        std::vector<std::uint8_t> args,
        float timeout_sec = 5.f
    );

    static std::uint32_t invoke_async(
        PeerId target,
        const std::string& name,
        std::function<void(BinaryReader&)> on_reply,
        std::function<void()> on_timeout,
        std::vector<std::uint8_t> args,
        float timeout_sec = 5.f
    );

    static void handle_call(PeerId sender, bool on_server, BinaryReader& r);
    static void handle_reply(BinaryReader& r);

    static void update(float dt);
    static void clear();

private:
    static std::uint32_t next_request_id();

    static void send(
        PeerId target,
        std::uint32_t rid,
        const std::string& name,
        const std::vector<std::uint8_t>& args
    );
    static void send(
        std::uint32_t rid,
        const std::string& name,
        const std::vector<std::uint8_t>& args
    );

    static std::unordered_map<std::string, RpcHandler>& handlers();
    static std::unordered_map<std::uint32_t, PendingRpc>& pending();

    inline static std::uint32_t _next_id = 1;
};

}