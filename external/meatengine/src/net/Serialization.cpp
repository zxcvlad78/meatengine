#include <meatengine/net/Serialization.hpp>
#include <algorithm>
#include <cstring>

namespace me::net {

BinaryWriter::BinaryWriter() : m_buffer() {}
BinaryWriter::BinaryWriter(size_t initial_capacity) {
    m_buffer.reserve(initial_capacity);
}

void BinaryWriter::write(int8_t v) { write(&v, 1); }
void BinaryWriter::write(uint8_t v) { write(&v, 1); }

void BinaryWriter::write(int16_t v) {
    uint16_t n = to_be(static_cast<uint16_t>(v));
    write(&n, sizeof(n));
}
void BinaryWriter::write(uint16_t v) {
    uint16_t n = to_be(v);
    write(&n, sizeof(n));
}
void BinaryWriter::write(int32_t v) {
    uint32_t n = to_be(static_cast<uint32_t>(v));
    write(&n, sizeof(n));
}
void BinaryWriter::write(uint32_t v) {
    uint32_t n = to_be(v);
    write(&n, sizeof(n));
}
void BinaryWriter::write(int64_t v) {
    uint64_t n = to_be(static_cast<uint64_t>(v));
    write(&n, sizeof(n));
}
void BinaryWriter::write(uint64_t v) {
    uint64_t n = to_be(v);
    write(&n, sizeof(n));
}

void BinaryWriter::write(float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    uint32_t n = to_be(bits);
    write(&n, sizeof(n));
}

void BinaryWriter::write(double v) {
    uint64_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    uint64_t n = to_be(bits);
    write(&n, sizeof(n));
}

void BinaryWriter::write(bool v) {
    uint8_t byte = v ? 1 : 0;
    write(&byte, 1);
}

void BinaryWriter::write(const std::string& str) {
    uint32_t len = static_cast<uint32_t>(str.size());
    write(len);
    if (len > 0) write(str.data(), len);
}

void BinaryWriter::write(const char* v) {
    if (!v) {
        write(std::string{});
        return;
    }
    const size_t n = std::strlen(v);
    write(static_cast<uint32_t>(n));
    if (n > 0) write(v, n);
}

void BinaryWriter::write(const std::vector<uint8_t>& v) {
    const uint32_t len = static_cast<uint32_t>(v.size());
    write(len);
    if (len > 0) write(v.data(), len);
}

void BinaryWriter::write(const void* data, size_t size) {
    if (size == 0) return;
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    m_buffer.insert(m_buffer.end(), bytes, bytes + size);
}

BinaryReader::BinaryReader(const uint8_t* data, size_t size)
    :
      m_data(data), m_size(size),
      m_pos(0), m_valid(true)
    {}

BinaryReader::BinaryReader(std::vector<uint8_t> buffer)
    :
      m_owned(std::move(buffer)), m_data(m_owned.data()),
      m_size(m_owned.size()), m_pos(0), m_valid(true)
    {}

bool BinaryReader::read(int8_t& out) { return read(&out, 1); }
bool BinaryReader::read(uint8_t& out) { return read(&out, 1); }

bool BinaryReader::read(int16_t& out) {
    uint16_t n;
    if (!read(&n, sizeof(n))) return false;
    out = static_cast<int16_t>(from_be(n));
    return true;
}
bool BinaryReader::read(uint16_t& out) {
    uint16_t n;
    if (!read(&n, sizeof(n))) return false;
    out = from_be(n);
    return true;
}
bool BinaryReader::read(int32_t& out) {
    uint32_t n;
    if (!read(&n, sizeof(n))) return false;
    out = static_cast<int32_t>(from_be(n));
    return true;
}
bool BinaryReader::read(uint32_t& out) {
    uint32_t n;
    if (!read(&n, sizeof(n))) return false;
    out = from_be(n);
    return true;
}
bool BinaryReader::read(int64_t& out) {
    uint64_t n;
    if (!read(&n, sizeof(n))) return false;
    out = static_cast<int64_t>(from_be(n));
    return true;
}
bool BinaryReader::read(uint64_t& out) {
    uint64_t n;
    if (!read(&n, sizeof(n))) return false;
    out = from_be(n);
    return true;
}

bool BinaryReader::read(float& out) {
    uint32_t n;
    if (!read(&n, sizeof(n))) return false;
    uint32_t host = from_be(n);
    std::memcpy(&out, &host, sizeof(host));
    return true;
}

bool BinaryReader::read(double& out) {
    uint64_t n;
    if (!read(&n, sizeof(n))) return false;
    uint64_t host = from_be(n);
    std::memcpy(&out, &host, sizeof(host));
    return true;
}

bool BinaryReader::read(bool& out) {
    uint8_t byte;
    if (!read(&byte, 1)) return false;
    out = (byte != 0);
    return true;
}

bool BinaryReader::read(std::string& out) {
    uint32_t len;
    if (!read(len)) return false;
    if (len == 0) {
        out.clear();
        return true;
    }
    if (len > m_size || m_pos > m_size - len) {
        m_valid = false;
        return false;
    }
    out.assign(reinterpret_cast<const char*>(m_data + m_pos), len);
    m_pos += len;
    return true;
}

bool BinaryReader::read(std::vector<uint8_t>& out) {
    uint32_t len;
    if (!read(len)) return false;
    if (len == 0) {
        out.clear();
        return true;
    }
    if (len > m_size || m_pos > m_size - len) {
        m_valid = false;
        return false;
    }
    out.resize(len);
    std::memcpy(out.data(), m_data + m_pos, len);
    m_pos += len;
    return true;
}

bool BinaryReader::read(void* out, size_t size) {
    if (size == 0) return true;
    if (size > m_size || m_pos > m_size - size) {
        m_valid = false;
        return false;
    }
    std::memcpy(out, m_data + m_pos, size);
    m_pos += size;
    return true;
}

}