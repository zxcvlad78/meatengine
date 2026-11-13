#pragma once

#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

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
    explicit BinaryWriter(std::size_t initial_capacity);
    explicit BinaryWriter(std::vector<std::uint8_t>* external);

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
    void write(std::string_view str);
    void write(const char* v);
    void write(const std::vector<uint8_t>& v);

    void write(const void* data, std::size_t size);

    void patch_u8(std::size_t off, uint8_t v);
    void patch_u16(std::size_t off, uint16_t v);
    void patch_u32(std::size_t off, uint32_t v);

    void reserve_front(std::size_t n) { target().resize(n, 0); }
    void reserve(std::size_t n) { target().reserve(n); }
    void reset() { target().clear(); }

    std::size_t size() const { return target().size(); }
    const std::uint8_t* data() const { return target().data(); }
    std::span<const std::uint8_t> view() const { return { target().data(), target().size() }; }

    const std::vector<std::uint8_t>& buffer() const& { return target(); }
    std::vector<std::uint8_t> buffer() && {
        if (m_external) return *m_external;
        return std::move(m_owned);
    }

    void bind(std::vector<std::uint8_t>* external) { m_external = external; }
    std::vector<std::uint8_t>* bound_buffer() const { return m_external; }

private:
    std::vector<std::uint8_t>& target() { return m_external ? *m_external : m_owned; }
    const std::vector<std::uint8_t>& target() const { return m_external ? *m_external : m_owned; }

    std::vector<std::uint8_t> m_owned;
    std::vector<std::uint8_t>* m_external = nullptr;
};

class BinaryReader {
public:
    BinaryReader(const uint8_t* data, std::size_t size);
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

    bool read(void* out, std::size_t size);

    bool skip(std::size_t n);
    bool seek(std::size_t pos);

    bool valid() const { return m_valid; }
    std::size_t remaining() const { return m_size - m_pos; }
    std::size_t position() const { return m_pos; }
    const std::uint8_t* data() const { return m_data; }
    std::size_t size() const { return m_size; }

private:
    std::vector<uint8_t> m_owned;
    const uint8_t* m_data = nullptr;
    std::size_t m_size = 0;
    std::size_t m_pos = 0;
    bool m_valid = true;
};

inline void write_varint(BinaryWriter& w, std::uint32_t v) {
    std::uint8_t buf[5];
    int n = 0;
    while (v >= 0x80) {
        buf[n++] = static_cast<std::uint8_t>((v & 0x7F) | 0x80);
        v >>= 7;
    }
    buf[n++] = static_cast<std::uint8_t>(v);
    w.write(buf, static_cast<std::size_t>(n));
}

inline bool read_varint(BinaryReader& r, std::uint32_t& out) {
    std::uint32_t result = 0;
    int shift = 0;
    for (int i = 0; i < 5; ++i) {
        std::uint8_t byte = 0;
        if (!r.read(byte)) return false;
        result |= static_cast<std::uint32_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            out = result;
            return true;
        }
        shift += 7;
    }
    return false;
}

inline void write_varint64(BinaryWriter& w, std::uint64_t v) {
    std::uint8_t buf[10];
    int n = 0;
    while (v >= 0x80) {
        buf[n++] = static_cast<std::uint8_t>((v & 0x7F) | 0x80);
        v >>= 7;
    }
    buf[n++] = static_cast<std::uint8_t>(v);
    w.write(buf, static_cast<std::size_t>(n));
}

inline bool read_varint64(BinaryReader& r, std::uint64_t& out) {
    std::uint64_t result = 0;
    int shift = 0;
    for (int i = 0; i < 10; ++i) {
        std::uint8_t byte = 0;
        if (!r.read(byte)) return false;
        result |= static_cast<std::uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            out = result;
            return true;
        }
        shift += 7;
    }
    return false;
}

inline void write_zigzag(BinaryWriter& w, std::int32_t v) {
    write_varint(w, static_cast<std::uint32_t>((v << 1) ^ (v >> 31)));
}

inline bool read_zigzag(BinaryReader& r, std::int32_t& out) {
    std::uint32_t raw = 0;
    if (!read_varint(r, raw)) return false;
    out = static_cast<std::int32_t>((raw >> 1) ^ (~(raw & 1) + 1));
    return true;
}

inline std::size_t varint_len(std::size_t v) {
    std::size_t n = 1;
    while (v >= 0x80) {
        v >>= 7;
        ++n;
    }
    return n;
}


template<typename T> void serialize(BinaryWriter& w, const T& v);
template<typename T> bool deserialize(BinaryReader& r, T& v);

namespace detail {

template<typename T>
inline constexpr bool is_builtin_arg_v =
    std::is_same_v<T, bool> ||
    std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t> ||
    std::is_same_v<T, std::int16_t> || std::is_same_v<T, std::uint16_t> ||
    std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t> ||
    std::is_same_v<T, std::int64_t> || std::is_same_v<T, std::uint64_t> ||
    std::is_same_v<T, float> || std::is_same_v<T, double>;

}

template<typename T>
inline void write_arg(BinaryWriter& w, const T& v) {
    using U = std::remove_cvref_t<T>;
    if constexpr (std::is_enum_v<U>) {
        w.write(static_cast<std::underlying_type_t<U>>(v));
    } else if constexpr (std::is_same_v<U, std::string>) {
        w.write(v);
    } else if constexpr (std::is_same_v<U, std::string_view>) {
        w.write(v);
    } else if constexpr (detail::is_builtin_arg_v<U>) {
        w.write(v);
    } else {
        serialize(w, v);
    }
}

template<typename T>
inline bool read_arg(BinaryReader& r, T& v) {
    using U = std::remove_cvref_t<T>;
    if constexpr (std::is_enum_v<U>) {
        std::underlying_type_t<U> raw{};
        if (!r.read(raw)) return false;
        v = static_cast<U>(raw);
        return true;
    } else if constexpr (std::is_same_v<U, std::string>) {
        return r.read(v);
    } else if constexpr (detail::is_builtin_arg_v<U>) {
        return r.read(v);
    } else {
        return deserialize(r, v);
    }
}

}
