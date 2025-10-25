#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>

namespace io_utils {
    void ensure_good_write(std::ostream& os);
    void ensure_good_read(std::istream& is);
}

inline void write_uint64(std::ostream& os, uint64_t value) {
    os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    io_utils::ensure_good_write(os);
}

inline uint64_t read_uint64(std::istream& is) {
    uint64_t value;
    is.read(reinterpret_cast<char*>(&value), sizeof(value));
    io_utils::ensure_good_read(is);
    return value;
}

inline void write_uint32(std::ostream& os, uint32_t value) {
    os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    io_utils::ensure_good_write(os);
}

inline uint32_t read_uint32(std::istream& is) {
    uint32_t value;
    is.read(reinterpret_cast<char*>(&value), sizeof(value));
    io_utils::ensure_good_read(is);
    return value;
}

inline void write_int32(std::ostream& os, int32_t value) {
    os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    io_utils::ensure_good_write(os);
}

inline int32_t read_int32(std::istream& is) {
    int32_t value;
    is.read(reinterpret_cast<char*>(&value), sizeof(value));
    io_utils::ensure_good_read(is);
    return value;
}

inline void write_string(std::ostream& os, const std::string& s) {
    uint32_t len = static_cast<uint32_t>(s.size());
    write_uint32(os, len);
    if (len > 0) {
        os.write(s.data(), len);
        io_utils::ensure_good_write(os);
    }
}

inline std::string read_string(std::istream& is) {
    uint32_t len = read_uint32(is);
    std::string s;
    if (len > 0) {
        s.resize(len);
        is.read(&s[0], len);
        io_utils::ensure_good_read(is);
    }
    return s;
}