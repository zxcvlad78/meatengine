#include <meatengine/td/Shader3D.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

#include <meatengine/td/ShaderOps.hpp>

namespace me::td {

    namespace {
        bool read_file(const std::string& path, std::string& out) {
            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "[td] Shader3D: cannot open '" << path << "'\n";
                return false;
            }
            std::ostringstream ss;
            ss << file.rdbuf();
            out = ss.str();
            return true;
        }
    }

    std::shared_ptr<Shader3D> Shader3D::operator()(const std::string& vertex_path,
                                                   const std::string& fragment_path) const {
        std::string vs, fs;
        if (!read_file(vertex_path, vs)) return nullptr;
        if (!read_file(fragment_path, fs)) return nullptr;

        if (!shader_compile(*program, vs, fs)) return nullptr;

        auto result = std::make_shared<Shader3D>();
        result->program = program;
        return result;
    }
}
