#pragma once

#include <glm/glm.hpp>
#include <string>

#include <meatengine/td/ShaderProgram.hpp>

namespace me::td {
    bool shader_compile(ShaderProgram& program,
                        const std::string& vertex_src,
                        const std::string& fragment_src,
                        std::string* error = nullptr);

    void shader_use(const ShaderProgram& program);

    GLint shader_uniform(const ShaderProgram& program, const std::string& name);

    void uniform_set_int(ShaderProgram& program, const std::string& name, int value);
    void uniform_set_float(ShaderProgram& program, const std::string& name, float value);
    void uniform_set_vec2(ShaderProgram& program, const std::string& name, const glm::vec2& value);
    void uniform_set_vec3(ShaderProgram& program, const std::string& name, const glm::vec3& value);
    void uniform_set_vec4(ShaderProgram& program, const std::string& name, const glm::vec4& value);
    void uniform_set_mat4(ShaderProgram& program, const std::string& name, const glm::mat4& value);
    void uniform_set_mat3(ShaderProgram& program, const std::string& name, const glm::mat3& value);
    void uniform_set_float_array(ShaderProgram& program, const std::string& name, const float* data, int count);
    void uniform_set_vec3_array(ShaderProgram& program, const std::string& name, const glm::vec3* data, int count);

    ShaderProgramRef compile_program(const std::string& vertex_src,
                                     const std::string& fragment_src);
}
