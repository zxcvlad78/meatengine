#pragma once
#include <cstdint>

#include <meatengine/meatnet/Types.hpp>

namespace me::cmp::net {
    struct Authority {
        me::net::ConnectionID id = 0;
    };
}