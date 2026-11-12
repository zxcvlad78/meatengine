#pragma once
#include <cstdint>

#include <meatengine/net/Protocol.hpp>

namespace me::net::cmp {
    struct Authority {
        me::net::PeerId id = 0;
    };
}