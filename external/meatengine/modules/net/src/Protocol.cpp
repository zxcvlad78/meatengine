#include <meatengine/net/Protocol.hpp>

#include <cstring>

namespace me::net {

void patch_envelope(
    std::uint8_t* dst,
    MsgType type,
    SendFlag flag,
    std::uint32_t seq
) {
    if (!dst) return;
    dst[0] = static_cast<std::uint8_t>(type);
    dst[1] = static_cast<std::uint8_t>(flag);
    const std::uint32_t s = to_be(seq);
    std::memcpy(dst + 2, &s, 4);
}

void write_envelope(BinaryWriter& w, const Envelope& env) {
    w.write(static_cast<std::uint8_t>(env.type));
    w.write(static_cast<std::uint8_t>(env.send_flag));
    w.write(env.seq);
}

bool read_envelope(BinaryReader& r, Envelope& out) {
    std::uint8_t t = 0, f = 0;
    if (!r.read(t)) return false;
    if (!r.read(f)) return false;
    if (!r.read(out.seq)) return false;
    out.type = static_cast<MsgType>(t);
    out.send_flag = static_cast<SendFlag>(f);
    return true;
}

}
