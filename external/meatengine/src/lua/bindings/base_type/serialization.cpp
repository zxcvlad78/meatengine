#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/meatnet/Serialization.hpp>

using me::net::BinaryReader;
using me::net::BinaryWriter;

static void bind(sol::state& lua) {
    lua.new_usertype<BinaryReader>("BinaryReader",
        sol::no_constructor,
        "ReadInt8", [](BinaryReader& r) { int8_t v; r.ReadInt8(v); return v; },
        "ReadUInt8", [](BinaryReader& r) { uint8_t v; r.ReadUInt8(v); return v; },
        "ReadInt16", [](BinaryReader& r) { int16_t v; r.ReadInt16(v); return v; },
        "ReadUInt16", [](BinaryReader& r) { uint16_t v; r.ReadUInt16(v); return v; },
        "ReadInt32", [](BinaryReader& r) { int32_t v; r.ReadInt32(v); return v; },
        "ReadUInt32", [](BinaryReader& r) { uint32_t v; r.ReadUInt32(v); return v; },
        "ReadInt64", [](BinaryReader& r) { int64_t v; r.ReadInt64(v); return v; },
        "ReadUInt64", [](BinaryReader& r) { uint64_t v; r.ReadUInt64(v); return v; },
        "ReadFloat", [](BinaryReader& r) { float v; r.ReadFloat(v); return v; },
        "ReadBool", [](BinaryReader& r) { bool v; r.ReadBool(v); return v; },
        "ReadString", [](BinaryReader& r) { std::string v; r.ReadString(v); return v; },
        "remaining", &BinaryReader::GetRemaining,
        "valid", &BinaryReader::IsValid
    );

    lua.new_usertype<BinaryWriter>("BinaryWriter",
        sol::constructors<BinaryWriter()>(),
        "WriteInt8", &BinaryWriter::WriteInt8,
        "WriteUInt8", &BinaryWriter::WriteUInt8,
        "WriteInt16", &BinaryWriter::WriteInt16,
        "WriteUInt16", &BinaryWriter::WriteUInt16,
        "WriteInt32", &BinaryWriter::WriteInt32,
        "WriteUInt32", &BinaryWriter::WriteUInt32,
        "WriteInt64", &BinaryWriter::WriteInt64,
        "WriteUInt64", &BinaryWriter::WriteUInt64,
        "WriteFloat", &BinaryWriter::WriteFloat,
        "WriteBool", &BinaryWriter::WriteBool,
        "WriteString", &BinaryWriter::WriteString,
        "size", [](BinaryWriter& w) { return w.Size(); },
        "clear", &BinaryWriter::Clear
    );
}

ME_LUA_BINDING(
    "engine:base_type:Serialization",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)