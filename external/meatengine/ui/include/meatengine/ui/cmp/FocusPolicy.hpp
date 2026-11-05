#pragma once
#include <cstdint>

namespace me::ui::cmp {
    struct FocusPolicy {
        enum class Kind : uint8_t { None, Block, Pass };
        Kind kind = Kind::Block;
    };
}