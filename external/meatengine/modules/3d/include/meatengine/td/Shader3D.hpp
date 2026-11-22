#pragma once

#include <memory>
#include <string>

#include <meatengine/td/ShaderProgram.hpp>

namespace me::td {
    struct Shader3D {
        using result_type = std::shared_ptr<Shader3D>;

        ShaderProgramRef program = std::make_shared<ShaderProgram>();

        std::shared_ptr<Shader3D> operator()(const std::string& vertex_path,
                                             const std::string& fragment_path) const;
    };

    using Shader3DRef = std::shared_ptr<Shader3D>;
}
