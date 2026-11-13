#include <meatengine/net/Packet.hpp>

#include <cstring>

namespace me::net {

namespace detail {

namespace {

constexpr std::size_t kPoolBlockSize = 64 * 1024;
constexpr std::size_t kPoolMaxBlocks = 512;

std::vector<std::vector<std::uint8_t>>& pool() {
    static std::vector<std::vector<std::uint8_t>> free_list;
    return free_list;
}

}

std::vector<std::uint8_t> acquire_buffer() {
    auto& list = pool();
    if (!list.empty()) {
        std::vector<std::uint8_t> buffer = std::move(list.back());
        list.pop_back();
        return buffer;
    }

    std::vector<std::uint8_t> buffer;
    buffer.reserve(kPoolBlockSize);
    return buffer;
}

void release_buffer(std::vector<std::uint8_t>&& buffer) {
    if (buffer.empty()) return;

    auto& list = pool();
    if (list.size() >= kPoolMaxBlocks) return;

    buffer.clear();
    list.push_back(std::move(buffer));
}

}

Packet::Packet()
    : m_buffer(detail::acquire_buffer())
    , m_writer(&m_buffer)
{
    m_writer.reserve(kEnvelopeHeaderSize + 2);
    begin(MsgType::Batch, SendFlag::Reliable);
}

Packet::~Packet() {
    detail::release_buffer(std::move(m_buffer));
}

Packet::Packet(Packet&& other) noexcept
    : m_buffer(std::move(other.m_buffer))
    , m_writer(std::move(other.m_writer))
    , m_type(other.m_type)
    , m_flag(other.m_flag)
    , m_count(other.m_count)
{
    m_writer.bind(&m_buffer);
}

Packet& Packet::operator=(Packet&& other) noexcept {
    if (this == &other) return *this;

    detail::release_buffer(std::move(m_buffer));

    m_buffer = std::move(other.m_buffer);
    m_writer = std::move(other.m_writer);
    m_writer.bind(&m_buffer);
    m_type = other.m_type;
    m_flag = other.m_flag;
    m_count = other.m_count;
    return *this;
}

void Packet::begin(MsgType type, SendFlag flag) {
    m_buffer.clear();
    m_buffer.resize(type == MsgType::Batch ? kEnvelopeHeaderSize + 1 : kEnvelopeHeaderSize, 0);
    m_buffer[0] = static_cast<std::uint8_t>(type);
    m_buffer[1] = static_cast<std::uint8_t>(flag);
    m_type = type;
    m_flag = flag;
    m_count = 0;
}

void Packet::record(MsgType type, SendFlag flag, const void* data, std::size_t size) {
    (void)flag;
    ensure_batch();
    commit_record();

    m_buffer.push_back(static_cast<std::uint8_t>(type));
    write_varint(m_writer, static_cast<std::uint32_t>(size));
    if (size > 0) m_writer.write(data, size);
}

void Packet::ensure_batch() {
    if (m_type == MsgType::Batch) return;

    const MsgType old_type = m_type;
    const SendFlag old_flag = m_flag;

    std::vector<std::uint8_t> old_body(
        m_buffer.begin() + static_cast<std::ptrdiff_t>(kEnvelopeHeaderSize),
        m_buffer.end()
    );

    m_buffer.clear();
    m_buffer.resize(kEnvelopeHeaderSize + 1, 0);
    m_buffer[0] = static_cast<std::uint8_t>(MsgType::Batch);
    m_buffer[1] = static_cast<std::uint8_t>(old_flag);
    m_type = MsgType::Batch;
    m_flag = old_flag;
    m_count = 0;

    if (!old_body.empty())
        record(old_type, old_flag, old_body.data(), old_body.size());
}

void Packet::commit_record() {
    const std::size_t prev_len = varint_len(m_count);
    ++m_count;
    const std::size_t new_len = varint_len(m_count);

    if (new_len > prev_len) {
        std::uint8_t zero = 0;
        m_buffer.insert(
            m_buffer.begin() + static_cast<std::ptrdiff_t>(kEnvelopeHeaderSize + prev_len),
            &zero,
            &zero + 1
        );
    }

    std::uint32_t x = static_cast<std::uint32_t>(m_count);
    std::size_t pos = kEnvelopeHeaderSize;
    while (x >= 0x80) {
        m_buffer[pos] = static_cast<std::uint8_t>((x & 0x7F) | 0x80);
        ++pos;
        x >>= 7;
    }
    m_buffer[pos] = static_cast<std::uint8_t>(x);
}

void Packet::clear() {
    begin(MsgType::Batch, m_flag);
}

}
