#pragma once

#include <meatengine/td/ShaderProgram.hpp>

namespace me::td {
    ShaderProgramRef& default_program();
    ShaderProgramRef& ensure_default_program();
}
