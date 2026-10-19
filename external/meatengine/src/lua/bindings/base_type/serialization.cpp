#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/net/Serialization.hpp>

using me::net::BinaryReader;
using me::net::BinaryWriter;

static void bind(sol::state& lua) {
    lua.new_usertype<BinaryReader>("BinaryReader",
        sol::no_constructor,
        "read_u8", [](BinaryReader& r) { uint8_t v; r.read(v); return v; },
        "read_i8", [](BinaryReader& r) { int8_t  v; r.read(v); return v; },
        "read_i16", [](BinaryReader& r) { int16_t v; r.read(v); return v; },
        "read_u16", [](BinaryReader& r) { uint16_t v; r.read(v); return v; },
        "read_i32", [](BinaryReader& r) { int32_t v; r.read(v); return v; },
        "read_u32", [](BinaryReader& r) { uint32_t v; r.read(v); return v; },
        "read_i64", [](BinaryReader& r) { int64_t v; r.read(v); return v; },
        "read_u64", [](BinaryReader& r) { uint64_t v; r.read(v); return v; },
        "read_f32", [](BinaryReader& r) { float v; r.read(v); return v; },
        "read_bool", [](BinaryReader& r) { bool v; r.read(v); return v; },
        "read_str", [](BinaryReader& r) { std::string v; r.read(v); return v; },
        "read_blob", [](BinaryReader& r) {
            std::vector<std::uint8_t> v;
            if (!r.read(v)) {
                throw std::runtime_error("BinaryReader:read_blob — truncated payload");
            }
            return v;
        },
        "remaining", &BinaryReader::remaining,
        "valid", &BinaryReader::valid
    );

    lua.new_usertype<BinaryWriter>("BinaryWriter",
        sol::constructors<BinaryWriter()>(),
        "write_i8",  [](BinaryWriter& w, int v) { w.write(static_cast<int8_t>(v));  },
        "write_i16", [](BinaryWriter& w, int v) { w.write(static_cast<int16_t>(v)); },
        "write_i32", [](BinaryWriter& w, int v) { w.write(static_cast<int32_t>(v)); },
        "write_i64", [](BinaryWriter& w, int64_t v) { w.write(v); },
        "write_u8",  [](BinaryWriter& w, int v) { w.write(static_cast<uint8_t>(v));  },
        "write_u16", [](BinaryWriter& w, int v) { w.write(static_cast<uint16_t>(v)); },
        "write_u32", [](BinaryWriter& w, uint32_t v) { w.write(v); },
        "write_u64", [](BinaryWriter& w, uint64_t v) { w.write(v); },
        "write_f32", [](BinaryWriter& w, float v) { w.write(v); },
        "write_f64", [](BinaryWriter& w, double v) { w.write(v); },
        "write_bool",[](BinaryWriter& w, bool v) { w.write(v); },
        "write_str", [](BinaryWriter& w, const std::string& v) { w.write(v); },
        "write_cstr", [](BinaryWriter& w, const char* v) { w.write(v); },
        "write_blob", [](BinaryWriter& w, const std::vector<uint8_t>& v) { w.write(v); },

        "buffer", [](BinaryWriter& w) { return w.buffer(); }
    );
}

ME_LUA_BINDING(
    "engine:base_type:Serialization",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)