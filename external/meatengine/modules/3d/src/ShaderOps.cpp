#include <meatengine/td/ShaderOps.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <iostream>

namespace me::td {

    namespace {
        GLuint compile_stage(GLenum stage, const std::string& src, std::string& log) {
            GLuint shader = glCreateShader(stage);
            const GLchar* src_ptr = src.c_str();
            glShaderSource(shader, 1, &src_ptr, nullptr);
            glCompileShader(shader);

            GLint ok = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
            if (!ok) {
                GLint len = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
                log.resize(static_cast<size_t>(len));
                glGetShaderInfoLog(shader, len, nullptr, log.data());
                glDeleteShader(shader);
                return 0;
            }
            return shader;
        }
    }

    bool shader_compile(ShaderProgram& program,
                        const std::string& vertex_src,
                        const std::string& fragment_src,
                        std::string* error) {
        if (program.handle) {
            glDeleteProgram(program.handle);
            program.handle = 0;
        }
        program.uniforms.clear();

        std::string log;
        GLuint vs = compile_stage(GL_VERTEX_SHADER, vertex_src, log);
        GLuint fs = vs ? compile_stage(GL_FRAGMENT_SHADER, fragment_src, log) : 0;

        if (!vs || !fs) {
            if (error) *error = log;
            std::cerr << "[td] shader compile error: " << log << '\n';
            if (vs) glDeleteShader(vs);
            if (fs) glDeleteShader(fs);
            return false;
        }

        GLuint prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);
        glDeleteShader(vs);
        glDeleteShader(fs);

        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            log.resize(static_cast<size_t>(len));
            glGetProgramInfoLog(prog, len, nullptr, log.data());
            if (error) *error = log;
            std::cerr << "[td] shader link error: " << log << '\n';
            glDeleteProgram(prog);
            return false;
        }

        program.handle = prog;
        return true;
    }

    void shader_use(const ShaderProgram& program) {
        glUseProgram(program.handle);
    }

    GLint shader_uniform(const ShaderProgram& program, const std::string& name) {
        auto& cache = const_cast<std::unordered_map<std::string, GLint>&>(program.uniforms);
        auto it = cache.find(name);
        if (it != cache.end()) return it->second;

        GLint location = glGetUniformLocation(program.handle, name.c_str());
        cache.emplace(name, location);
        return location;
    }

    void uniform_set_int(ShaderProgram& program, const std::string& name, int value) {
        GLint loc = shader_uniform(program, name);
        if (loc >= 0) glUniform1i(loc, value);
    }

    void uniform_set_float(ShaderProgram& program, const std::string& name, float value) {
        GLint loc = shader_uniform(program, name);
        if (loc >= 0) glUniform1f(loc, value);
    }

    void uniform_set_vec2(ShaderProgram& program, const std::string& name, const glm::vec2& value) {
        GLint loc = shader_uniform(program, name);
        if (loc >= 0) glUniform2fv(loc, 1, glm::value_ptr(value));
    }

    void uniform_set_vec3(ShaderProgram& program, const std::string& name, const glm::vec3& value) {
        GLint loc = shader_uniform(program, name);
        if (loc >= 0) glUniform3fv(loc, 1, glm::value_ptr(value));
    }

    void uniform_set_vec4(ShaderProgram& program, const std::string& name, const glm::vec4& value) {
        GLint loc = shader_uniform(program, name);
        if (loc >= 0) glUniform4fv(loc, 1, glm::value_ptr(value));
    }

    void uniform_set_mat4(ShaderProgram& program, const std::string& name, const glm::mat4& value) {
        GLint loc = shader_uniform(program, name);
        if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }

    void uniform_set_mat3(ShaderProgram& program, const std::string& name, const glm::mat3& value) {
        GLint loc = shader_uniform(program, name);
        if (loc >= 0) glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }

    void uniform_set_float_array(ShaderProgram& program, const std::string& name, const float* data, int count) {
        GLint loc = shader_uniform(program, name);
        if (loc >= 0) glUniform1fv(loc, count, data);
    }

    void uniform_set_vec3_array(ShaderProgram& program, const std::string& name, const glm::vec3* data, int count) {
        GLint loc = shader_uniform(program, name);
        if (loc >= 0) glUniform3fv(loc, count, glm::value_ptr(*data));
    }

    ShaderProgramRef compile_program(const std::string& vertex_src, const std::string& fragment_src) {
        auto program = std::make_shared<ShaderProgram>();
        if (!shader_compile(*program, vertex_src, fragment_src)) return nullptr;
        return program;
    }
}
