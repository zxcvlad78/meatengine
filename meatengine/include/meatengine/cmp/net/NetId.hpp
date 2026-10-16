#pragma once
#include <cstdint>

namespace me::cmp::net {
    struct NetId {
        std::uint64_t value;

        NetId() {
            value = next();
        }

        static std::uint64_t next() {
            static std::uint64_t next_; 
            return next_++;
        }
    };
}