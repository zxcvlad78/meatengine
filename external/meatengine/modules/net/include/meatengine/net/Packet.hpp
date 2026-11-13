#pragma once

#include <cstdint>
#include <vector>

#include <meatengine/net/Protocol.hpp>
#include <meatengine/net/Serialization.hpp>

namespace me::net {

namespace detail {

std::vector<std::uint8_t> acquire_buffer();
void release_buffer(std::vector<std::uint8_t>&& buffer);

}

class Packet {
public:
    Packet();
    ~Packet();

    Packet(Packet&& other) noexcept;
    Packet& operator=(Packet&& other) noexcept;

    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;

    void begin(MsgType type, SendFlag flag);

    void record(MsgType type, SendFlag flag, const void* data, std::size_t size);
    template<typename F>
    void record(MsgType type, SendFlag flag, F&& fn) {
        (void)flag;
        ensure_batch();
        const std::size_t payload_start = m_buffer.size();
        fn(m_writer);
        const std::size_t payload_size = m_buffer.size() - payload_start;

        std::uint32_t x = static_cast<std::uint32_t>(payload_size);
        std::uint8_t tmp[5];
        int n = 0;
        while (x >= 0x80) {
            tmp[n++] = static_cast<std::uint8_t>((x & 0x7F) | 0x80);
            x >>= 7;
        }
        tmp[n++] = static_cast<std::uint8_t>(x);

        std::uint8_t framing[6];
        int flen = 0;
        framing[flen++] = static_cast<std::uint8_t>(type);
        for (int i = 0; i < n; ++i) framing[flen++] = tmp[i];
        m_buffer.insert(m_buffer.begin() + static_cast<std::ptrdiff_t>(payload_start),
                        framing, framing + flen);

        commit_record();
    }

    BinaryWriter& writer() { return m_writer; }

    MsgType type() const { return m_type; }
    SendFlag flag() const { return m_flag; }
    bool is_batch() const { return m_type == MsgType::Batch; }
    std::size_t record_count() const { return m_count; }

    const std::uint8_t* data() const { return m_buffer.data(); }
    std::uint8_t* header() { return m_buffer.data(); }
    std::size_t size() const { return m_buffer.size(); }
    const std::uint8_t* body_data() const { return m_buffer.data() + kEnvelopeHeaderSize; }
    std::size_t body_size() const { return m_buffer.size() - kEnvelopeHeaderSize; }

    bool empty() const { return m_buffer.size() <= kEnvelopeHeaderSize; }

    void clear();

    void reserve(std::size_t bytes) { m_buffer.reserve(bytes); }

private:
    void ensure_batch();
    void commit_record();

    std::vector<std::uint8_t> m_buffer;
    BinaryWriter m_writer;
    MsgType m_type = MsgType::Batch;
    SendFlag m_flag = SendFlag::Reliable;
    std::size_t m_count = 0;
};

}
