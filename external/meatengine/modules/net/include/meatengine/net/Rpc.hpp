#pragma once

#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <meatengine/net/Network.hpp>
#include <meatengine/net/Packet.hpp>
#include <meatengine/net/Protocol.hpp>
#include <meatengine/net/Serialization.hpp>

namespace me::net {

struct RpcContext {
    PeerId sender = kInvalidPeerId;
    bool from_server = false;
    std::uint32_t request_id = 0;

    std::vector<std::uint8_t> reply_payload;
    bool has_reply = false;
    PeerId reply_target = kInvalidPeerId;

    template<typename... Args>
    void reply(Args&&... args) {
        BinaryWriter w;
        (write_arg(w, args), ...);
        reply_payload = std::move(w).buffer();
        has_reply = true;
    }

    template<typename... Args>
    void reply_to(PeerId target, Args&&... args) {
        reply(std::forward<Args>(args)...);
        reply_target = target;
    }

    void reply_empty() {
        reply_payload.clear();
        has_reply = true;
    }
};

using RpcHandler = std::function<void(RpcContext&, BinaryReader&)>;
using RpcReplyCallback = std::function<void(BinaryReader&)>;
using RpcTimeoutCallback = std::function<void()>;

struct PendingRpc {
    RpcReplyCallback on_reply;
    RpcTimeoutCallback on_timeout;
    float deadline = 0.f;
};

namespace detail {

template<typename T>
struct fn_traits {
    using args = std::tuple<>;
    static constexpr bool valid = false;
};

template<typename R, typename... A>
struct fn_traits<R(*)(A...)> { using args = std::tuple<A...>; static constexpr bool valid = true; };
template<typename R, typename... A>
struct fn_traits<R(*)(A...) noexcept> { using args = std::tuple<A...>; static constexpr bool valid = true; };
template<typename R, typename C, typename... A>
struct fn_traits<R(C::*)(A...)> { using args = std::tuple<A...>; static constexpr bool valid = true; };
template<typename R, typename C, typename... A>
struct fn_traits<R(C::*)(A...) const> { using args = std::tuple<A...>; static constexpr bool valid = true; };
template<typename R, typename C, typename... A>
struct fn_traits<R(C::*)(A...) noexcept> { using args = std::tuple<A...>; static constexpr bool valid = true; };
template<typename R, typename C, typename... A>
struct fn_traits<R(C::*)(A...) const noexcept> { using args = std::tuple<A...>; static constexpr bool valid = true; };

template<typename T, typename = void>
struct lambda_args : fn_traits<T> {};

template<typename T>
struct lambda_args<T, std::void_t<decltype(&std::remove_cvref_t<T>::operator())>>
    : fn_traits<decltype(&std::remove_cvref_t<T>::operator())> {};

template<typename T>
using lambda_args_t = typename lambda_args<T>::args;

template<typename Args>
inline constexpr std::size_t stream_arg_count_v =
    std::tuple_size_v<Args> > 0 ? std::tuple_size_v<Args> - 1 : 0;

template<typename T>
inline constexpr bool is_writer_functor_v =
    std::is_invocable_v<std::remove_cvref_t<T>&, BinaryWriter&>;

inline void report_bad_payload(std::string_view name) {
    static std::uint64_t counter = 0;
    if ((++counter % 64) == 1)
        std::printf("[Rpc] payload of '%.*s' does not match the handler signature\n",
                    static_cast<int>(name.size()), name.data());
}

}

class Rpc {
public:
    Rpc() = delete;


    static void bind(std::string_view name, RpcHandler handler);

    template<typename F>
    static void bind(std::string_view name, F&& handler) {
        using Args = detail::lambda_args_t<std::remove_cvref_t<F>>;
        static_assert(detail::lambda_args<std::remove_cvref_t<F>>::valid,
                      "Rpc::bind: the handler must be a non generic lambda or a function pointer");
        bind_typed(name, std::forward<F>(handler),
                   std::make_index_sequence<detail::stream_arg_count_v<Args>>{});
    }

    static bool has(std::string_view name);
    static RpcId id(std::string_view name);
    static std::string_view name_of(RpcId id);
    static void unbind(std::string_view name);
    static void clear_handlers();


    template<typename... Args>
    static void invoke(std::string_view name, Args&&... args) {
        invoke_to(kServerPeerId, name, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void invoke_to(PeerId target, std::string_view name, Args&&... args) {
        const RpcId rpc = rpc_id(name);
        Network::send(target, MsgType::RpcCall, SendFlag::Reliable, [&](BinaryWriter& w) {
            write_call(w, rpc, 0);
            (write_arg(w, args), ...);
        });
        Network::stats().rpcs_sent++;
    }

    template<typename... Args>
    static void invoke_unreliable(std::string_view name, Args&&... args) {
        invoke_unreliable_to(kServerPeerId, name, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void invoke_unreliable_to(PeerId target, std::string_view name, Args&&... args) {
        const RpcId rpc = rpc_id(name);
        Network::send(target, MsgType::RpcCall, SendFlag::UnreliableNoDelay, [&](BinaryWriter& w) {
            write_call(w, rpc, 0);
            (write_arg(w, args), ...);
        });
        Network::stats().rpcs_sent++;
    }

    template<typename... Args>
    static void broadcast(std::string_view name, Args&&... args) {
        const RpcId rpc = rpc_id(name);

        if constexpr (sizeof...(Args) == 1 && detail::is_writer_functor_v<Args...>) {
            auto tuple = std::forward_as_tuple(args...);
            auto&& writer = std::get<0>(tuple);

            if (Network::is_server())
                run_local_raw(rpc, writer);

            Network::send(MsgType::RpcCall, SendFlag::Reliable, [&](BinaryWriter& w) {
                write_call(w, rpc, 0);
                writer(w);
            });
        } else {
            if (Network::is_server())
                run_local(rpc, args...);

            Network::send(MsgType::RpcCall, SendFlag::Reliable, [&](BinaryWriter& w) {
                write_call(w, rpc, 0);
                (write_arg(w, args), ...);
            });
        }

        Network::stats().rpcs_sent++;
    }

    template<typename... Args>
    static void invoke(Packet& packet, std::string_view name, Args&&... args) {
        invoke(packet, rpc_id(name), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void invoke(Packet& packet, RpcId id, Args&&... args) {
        packet.record(MsgType::RpcCall, SendFlag::Reliable, [&](BinaryWriter& w) {
            write_call(w, id, 0);
            (write_arg(w, args), ...);
        });
        Network::stats().rpcs_sent++;
    }


    static std::uint32_t invoke_async(
        std::string_view name,
        RpcReplyCallback on_reply,
        RpcTimeoutCallback on_timeout,
        std::vector<std::uint8_t> args,
        float timeout_sec = 5.f);

    static std::uint32_t invoke_async(
        PeerId target,
        std::string_view name,
        RpcReplyCallback on_reply,
        RpcTimeoutCallback on_timeout,
        std::vector<std::uint8_t> args,
        float timeout_sec = 5.f);

    template<typename... Args>
    static std::uint32_t invoke_async(
        std::string_view name,
        RpcReplyCallback on_reply,
        RpcTimeoutCallback on_timeout,
        float timeout_sec,
        Args&&... args)
    {
        return invoke_async_to(
            kServerPeerId, name, std::move(on_reply), std::move(on_timeout), timeout_sec,
            std::forward<Args>(args)...
        );
    }

    template<typename... Args>
    static std::uint32_t invoke_async_to(
        PeerId target,
        std::string_view name,
        RpcReplyCallback on_reply,
        RpcTimeoutCallback on_timeout,
        float timeout_sec,
        Args&&... args)
    {
        const RpcId rpc = rpc_id(name);
        const std::uint32_t rid = next_request_id();

        pending()[rid] = PendingRpc{
            std::move(on_reply),
            std::move(on_timeout),
            timeout_sec
        };

        Network::send(target, MsgType::RpcCall, SendFlag::Reliable, [&](BinaryWriter& w) {
            write_call(w, rpc, rid);
            (write_arg(w, args), ...);
        });
        Network::stats().rpcs_sent++;
        return rid;
    }


    static void handle_call(PeerId sender, bool on_server, BinaryReader& r);
    static void handle_reply(BinaryReader& r);

    static void dispatch_local(RpcId id, BinaryReader& r);

    static void update(float dt);
    static void clear();

    static void write_call(BinaryWriter& w, RpcId id, std::uint32_t request_id) {
        w.write(id);
        w.write(request_id);
    }

private:
    template<typename F, std::size_t... I>
    static void bind_typed(std::string_view name, F&& handler, std::index_sequence<I...>) {
        using Args = detail::lambda_args_t<std::remove_cvref_t<F>>;

        bind_raw(name, [handler = std::forward<F>(handler),
                        rpc_name = std::string(name)](RpcContext& ctx, BinaryReader& r) mutable {
            constexpr std::size_t N = std::tuple_size_v<Args>;

            if constexpr (N <= 1) {
                handler(ctx);
            } else if constexpr (N == 2 && std::is_same_v<
                                   std::remove_cvref_t<std::tuple_element_t<1, Args>>,
                                   BinaryReader>) {
                handler(ctx, r);
            } else {
                std::tuple<std::decay_t<std::tuple_element_t<I + 1, Args>>...> values{};
                bool ok = true;
                std::apply([&](auto&... v) { ((ok = ok && read_arg(r, v)), ...); }, values);
                if (!ok) {
                    detail::report_bad_payload(rpc_name);
                    return;
                }
                std::apply([&](auto&... v) { handler(ctx, std::move(v)...); }, values);
            }
        });
    }

    static void bind_raw(std::string_view name, RpcHandler handler);

    template<typename... Args>
    static void run_local(RpcId id, Args&&... args) {
        BinaryWriter w;
        (write_arg(w, args), ...);
        auto payload = std::move(w).buffer();

        BinaryReader r(payload.data(), payload.size());
        dispatch_local(id, r);
    }

    template<typename F>
    static void run_local_raw(RpcId id, F& writer) {
        BinaryWriter w;
        writer(w);
        auto payload = std::move(w).buffer();

        BinaryReader r(payload.data(), payload.size());
        dispatch_local(id, r);
    }

    static std::unordered_map<RpcId, RpcHandler>& handlers();
    static std::unordered_map<RpcId, std::string>& names();
    static std::unordered_map<std::uint32_t, PendingRpc>& pending();

    static std::uint32_t next_request_id();
    inline static std::uint32_t _next_id = 1;
};

}
