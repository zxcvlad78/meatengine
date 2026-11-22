#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <glad/glad.h>

namespace me::td {

    // Plain data: GL program handle + uniform location cache.
    // Behavior lives in ShaderOps (free functions).
    struct ShaderProgram {
        GLuint handle = 0;
        std::unordered_map<std::string, GLint> uniforms;
    };

    using ShaderProgramRef = std::shared_ptr<ShaderProgram>;
}
