#pragma once
#include <cstdint>

#include <meatengine/net/Protocol.hpp>

namespace me::cmp::net {
    struct Authority {
        me::net::PeerId id = 0;
    };
}