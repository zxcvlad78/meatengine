#pragma once
#include <cstdint>

namespace godlike {

struct ItemStack {
    uint16_t def_idx = 0;
    uint32_t count = 0;

    bool empty() const noexcept { return count == 0; }
};

};