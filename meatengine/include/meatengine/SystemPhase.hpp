#pragma once
#include <cstdint>
#include <string_view>

namespace me {

enum class SystemPhase : uint8_t {
    Input,
    FixedUpdate,
    Update,
    Render,
    PostRender,
    Invalid
};

inline const char* to_string(SystemPhase p) {
    switch (p) {
        case SystemPhase::Input: return "input";
        case SystemPhase::FixedUpdate: return "fixed_update";
        case SystemPhase::Update: return "update";
        case SystemPhase::Render: return "render";
        case SystemPhase::PostRender: return "post_render";
        default: return "invalid";
    }
}

inline SystemPhase from_string(std::string_view s) {
    if (s == "input") return SystemPhase::Input;
    if (s == "fixed_update") return SystemPhase::FixedUpdate;
    if (s == "update") return SystemPhase::Update;
    if (s == "render") return SystemPhase::Render;
    if (s == "post_render") return SystemPhase::PostRender;
    return SystemPhase::Invalid;
}

}