#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

namespace me::net {

static inline uint16_t swap16(uint16_t v) {
    return static_cast<uint16_t>(((v & 0xFF) << 8) | ((v >> 8) & 0xFF));
}
static inline uint32_t swap32(uint32_t v) {
    return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) |
           ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
}
static inline uint64_t swap64(uint64_t v) {
    return (static_cast<uint64_t>(swap32(static_cast<uint32_t>(v))) << 32) | 
            swap32(static_cast<uint32_t>(v >> 32));
}

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static inline uint16_t to_be(uint16_t v) { return v; }
static inline uint32_t to_be(uint32_t v) { return v; }
static inline uint64_t to_be(uint64_t v) { return v; }
static inline uint16_t from_be(uint16_t v) { return v; }
static inline uint32_t from_be(uint32_t v) { return v; }
static inline uint64_t from_be(uint64_t v) { return v; }
#else
static inline uint16_t to_be(uint16_t v) { return swap16(v); }
static inline uint32_t to_be(uint32_t v) { return swap32(v); }
static inline uint64_t to_be(uint64_t v) { return swap64(v); }
static inline uint16_t from_be(uint16_t v) { return swap16(v); }
static inline uint32_t from_be(uint32_t v) { return swap32(v); }
static inline uint64_t from_be(uint64_t v) { return swap64(v); }
#endif

class BinaryWriter {
public:
    BinaryWriter() = default;
    explicit BinaryWriter(size_t initial_capacity);

    void write(int8_t v);
    void write(uint8_t v);
    void write(int16_t v);
    void write(uint16_t v);
    void write(int32_t v);
    void write(uint32_t v);
    void write(int64_t v);
    void write(uint64_t v);
    void write(float v);
    void write(double v);
    void write(bool v);
    void write(const std::string& str);
    void write(const char* v);
    void write(const std::vector<uint8_t>& v);

    void write(const void* data, size_t size); // write raw data

    void patch_u8(size_t off, uint8_t v);
    void patch_u16(size_t off, uint16_t v);
    void patch_u32(size_t off, uint32_t v);

    void reserve_front(size_t n) { m_buffer.resize(n, 0); }

    const std::vector<std::uint8_t>& buffer() const& { return m_buffer; }
    std::vector<std::uint8_t> buffer() && { return std::move(m_buffer); }

private:
    std::vector<uint8_t> m_buffer;
};

class BinaryReader {
public:
    BinaryReader(const uint8_t* data, size_t size);
    BinaryReader(std::vector<uint8_t> buffer);

    bool read(int8_t& out);
    bool read(uint8_t& out);
    bool read(int16_t& out);
    bool read(uint16_t& out);
    bool read(int32_t& out);
    bool read(uint32_t& out);
    bool read(int64_t& out);
    bool read(uint64_t& out);
    bool read(float& out);
    bool read(double& out);
    bool read(bool& out);
    bool read(std::string& out);
    bool read(std::vector<uint8_t>& out);

    bool read(void* out, size_t size); // read raw data

    bool valid() const { return m_valid; }
    size_t remaining() const { return m_size - m_pos; }
    size_t position() const { return m_pos; }

private:
    std::vector<uint8_t> m_owned; 
    const uint8_t* m_data = nullptr;
    size_t m_size = 0;
    size_t m_pos = 0;
    bool m_valid = true;
};

} // namespace MeatNet