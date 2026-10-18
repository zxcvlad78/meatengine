#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/meatnet/Serialization.hpp>
#include <meatengine/meatnet/Rpc.hpp>

static void bind(sol::state& lua) {
    sol::table rpc = lua.create_table();
    lua["Rpc"] = rpc;

    rpc.set_function("bind",
        [](const std::string& name, sol::protected_function fn) {
            me::net::Rpc::bind(name,
                [name, fn](me::net::RpcContext& ctx, me::net::BinaryReader& r) {
                    sol::state_view lua(fn.lua_state());
                    sol::table args = lua.create_table();
                    int i = 1;
                    while (r.GetRemaining() > 0) {
                        uint8_t t = 0;
                        if (!r.ReadUInt8(t)) break;
                        switch (t) {
                            case 0: { int32_t v; r.ReadInt32(v); args[i++] = v; break; }
                            case 1: { uint32_t v; r.ReadUInt32(v); args[i++] = v; break; }
                            case 2: { float v; r.ReadFloat(v); args[i++] = v; break; }
                            case 3: { bool v; r.ReadBool(v); args[i++] = v; break; }
                            case 4: { std::string v; r.ReadString(v); args[i++] = v; break; }
                            default: return;
                        }
                    }
                    auto res = fn(args, ctx.request_id, ctx.sender);
                    if (!res.valid()) {
                        sol::error e = res;
                        std::cerr << "[Rpc.bind '" << name << "'] " << e.what() << "\n";
                    }
                });
        });

    rpc.set_function("invoke",
        [](const std::string& name, sol::variadic_args va) {
            me::net::BinaryWriter w;
            for (auto v : va) {
                auto t = v.get_type();
                if (t == sol::type::number) {
                    if (v.is<int>()) { w.WriteUInt8(0); w.WriteInt32(v.as<int>()); }
                    else { w.WriteUInt8(2); w.WriteFloat(v.as<float>()); }
                } else if (t == sol::type::boolean) { w.WriteUInt8(3); w.WriteBool(v.as<bool>()); }
                else if (t == sol::type::string) { w.WriteUInt8(4); w.WriteString(v.as<std::string>()); }
            }
            me::net::Rpc::invoke(name, w.GetBuffer());
        });

    rpc.set_function("invoke_on",
        [](me::net::PeerId target, const std::string& name, sol::variadic_args va) {
            me::net::BinaryWriter w;
            for (auto v : va) {
                auto t = v.get_type();
                if (t == sol::type::number) {
                    if (v.is<int>())        { w.WriteUInt8(0); w.WriteInt32(v.as<int>()); }
                    else                    { w.WriteUInt8(2); w.WriteFloat(v.as<float>()); }
                } else if (t == sol::type::boolean) { w.WriteUInt8(3); w.WriteBool(v.as<bool>()); }
                else if (t == sol::type::string)    { w.WriteUInt8(4); w.WriteString(v.as<std::string>()); }
            }
            me::net::Rpc::invoke(target, name, w.GetBuffer());
        });

    rpc.set_function("has",
        [](const std::string& name) { return me::net::Rpc::has(name); });
}

ME_LUA_BINDING(
    "engine:api:Rpc",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)