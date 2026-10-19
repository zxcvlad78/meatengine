#pragma once
#include <cstdint>

namespace me::cmp::ui {

    struct Val {
        enum class Kind : uint8_t { Auto, Px, Percent };
        Kind kind = Kind::Auto;
        float value = 0.f;

        static Val Auto() { return {Kind::Auto, 0.f}; }
        static Val Px(float v) { return {Kind::Px, v}; }
        static Val Percent(float v) { return {Kind::Percent, v}; }

        bool is_auto() const { return kind == Kind::Auto; }
    };

}