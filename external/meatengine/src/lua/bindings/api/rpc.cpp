#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/net/Rpc.hpp>
#include <meatengine/net/Serialization.hpp>
#include <meatengine/net/Network.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {

using me::net::RpcContext;
using me::net::BinaryReader;
using me::net::BinaryWriter;

void write_lua_value(BinaryWriter& w, const sol::object& v) {
    switch (v.get_type()) {
        case sol::type::number: {
            double d = v.as<double>();
            if (!std::isfinite(d)) {
                w.write(static_cast<float>(d));
                return;
            }

            const bool is_integral = (d == std::floor(d));

            if (is_integral && d >= 0.0 &&
                d <= static_cast<double>(std::numeric_limits<uint32_t>::max()))
            {
                w.write(static_cast<uint32_t>(d));
                return;
            }

            if (is_integral &&
                d >= static_cast<double>(std::numeric_limits<int32_t>::min()) &&
                d < 0.0)
            {
                w.write(static_cast<int32_t>(d));
                return;
            }

            if (is_integral && std::abs(d) < 9.0e18) {
                w.write(static_cast<int64_t>(d));
                return;
            }

            w.write(static_cast<float>(d));
            return;
        }
        case sol::type::boolean: w.write(v.as<bool>()); return;
        case sol::type::string: {
            std::string_view sv = v.as<std::string_view>();
            w.write(static_cast<uint32_t>(sv.size()));
            if (!sv.empty()) w.write(sv.data(), sv.size());
            return;
        }
        default:
            throw std::runtime_error("unsupported Lua type (only int, float, bool, string)");
    }
}

}

static void bind(sol::state& lua) {
    auto ctx_ut = lua.new_usertype<RpcContext>("RpcContext",
        sol::no_constructor,
        "sender", sol::readonly(&RpcContext::sender),
        "from_server", sol::readonly(&RpcContext::from_server),
        "request_id", sol::readonly(&RpcContext::request_id),
        "has_reply", sol::readonly(&RpcContext::has_reply)
    );

    ctx_ut.set_function("reply",
        [](RpcContext& ctx, sol::variadic_args va) {
            BinaryWriter w;
            for (auto v : va) write_lua_value(w, v);
            ctx.reply_payload = std::move(w).buffer();
            ctx.has_reply = true;
        });

    ctx_ut.set_function("reply_empty", [](RpcContext& ctx) {
        ctx.reply_payload.clear();
        ctx.has_reply = true;
    });

    sol::table rpc = lua.create_table();
    lua["Rpc"] = rpc;

    rpc.set_function("bind",
        [](const std::string& name, sol::protected_function fn) {
            me::net::Rpc::bind(name,
                [name, fn](RpcContext& ctx, BinaryReader& r) {
                    auto res = fn(&ctx, &r);
                    if (!res.valid()) {
                        sol::error e = res;
                        std::cerr << "[Rpc.bind '" << name << "'] "
                                  << e.what() << "\n";
                    }
                });
        });

    rpc.set_function("invoke",
        [](const std::string& name, sol::variadic_args va) {
            me::net::Network::send(
                me::net::MsgType::RpcCall,
                me::net::SendFlag::Reliable,
                [&](BinaryWriter& w) {
                    w.write(static_cast<std::uint32_t>(0));
                    w.write(name);
                    for (auto v : va) write_lua_value(w, v);
                });
        });

    rpc.set_function("invoke_on",
        [](me::net::PeerId target, const std::string& name, sol::variadic_args va) {
            me::net::Network::send(
                target,
                me::net::MsgType::RpcCall,
                me::net::SendFlag::Reliable,
                [&](BinaryWriter& w) {
                    w.write(static_cast<std::uint32_t>(0));
                    w.write(name);
                    for (auto v : va) write_lua_value(w, v);
                });
        });

    rpc.set_function("broadcast", [](const std::string& name, sol::variadic_args va) {
        me::net::Rpc::broadcast(name,
            [&](BinaryWriter& w) {
                for (auto v : va) write_lua_value(w, v);
            });
    });

    rpc.set_function("invoke_async",
        [](const std::string& name,
           sol::protected_function on_reply,
           sol::protected_function on_timeout,
           float timeout_sec,
           sol::variadic_args va
        ) {
            BinaryWriter w;
            for (auto v : va) write_lua_value(w, v);

            me::net::Rpc::invoke_async(
                name,
                [on_reply](BinaryReader& rr) {
                    auto res = on_reply(&rr);
                    if (!res.valid()) {
                        sol::error e = res;
                        std::cerr << "[Rpc.reply] " << e.what() << "\n";
                    }
                },
                [on_timeout]() {
                    auto res = on_timeout();
                    if (!res.valid()) {
                        sol::error e = res;
                        std::cerr << "[Rpc.timeout] " << e.what() << "\n";
                    }
                },
                std::move(w).buffer(),
                timeout_sec);
        }
    );

    rpc.set_function("invoke_on_async",
        [](me::net::PeerId target,
           const std::string& name,
           sol::protected_function on_reply,
           sol::protected_function on_timeout,
           float timeout_sec,
           sol::variadic_args va
        ) {
            BinaryWriter w;
            for (auto v : va) write_lua_value(w, v);

            me::net::Rpc::invoke_async(
                target,
                name,
                [on_reply](BinaryReader& rr) {
                    auto res = on_reply(&rr);
                    if (!res.valid()) {
                        sol::error e = res;
                        std::cerr << "[Rpc.reply] " << e.what() << "\n";
                    }
                },
                [on_timeout]() {
                    auto res = on_timeout();
                    if (!res.valid()) {
                        sol::error e = res;
                        std::cerr << "[Rpc.timeout] " << e.what() << "\n";
                    }
                },
                std::move(w).buffer(),
                timeout_sec);
        }
    );

    rpc.set_function("has", [](const std::string& name) { return me::net::Rpc::has(name); });
}

ME_LUA_BINDING(
    "engine:api:Rpc",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)