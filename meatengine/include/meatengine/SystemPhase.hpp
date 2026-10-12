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
        case SystemPhase::Input: return "Input";
        case SystemPhase::FixedUpdate: return "FixedUpdate";
        case SystemPhase::Update: return "Update";
        case SystemPhase::Render: return "Render";
        case SystemPhase::PostRender: return "PostRender";
        default: return "Invalid";
    }
}

inline SystemPhase from_string(std::string_view s) {
    if (s == "Input") return SystemPhase::Input;
    if (s == "FixedUpdate") return SystemPhase::FixedUpdate;
    if (s == "Update") return SystemPhase::Update;
    if (s == "Render") return SystemPhase::Render;
    if (s == "PostRender") return SystemPhase::PostRender;
    return SystemPhase::Invalid;
}

}