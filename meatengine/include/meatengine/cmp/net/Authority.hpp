#pragma once
#include <cstdint>

#include <meatengine/meatnet/Protocol.hpp>

namespace me::cmp::net {
    struct Authority {
        me::net::PeerId id = 0;
    };
}