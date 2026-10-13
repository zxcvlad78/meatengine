#pragma once
#include <cstdint>

namespace me::cmp::ui {
    struct FocusPolicy {
        enum class Kind : uint8_t { None, Block, Pass };
        Kind kind = Kind::Block;
    };
}