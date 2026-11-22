#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace me::td {
    struct MeshVertex {
        glm::vec3 position{0.f};
        glm::vec3 normal{0.f, 1.f, 0.f};
        glm::vec2 uv{0.f};
    };

    struct Mesh {
        using result_type = std::shared_ptr<Mesh>;

        std::vector<MeshVertex> vertices;
        std::vector<uint32_t> indices;

        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        uint32_t index_count = 0;

        glm::vec3 aabb_min{0.f};
        glm::vec3 aabb_max{0.f};
        bool has_aabb = false;

        std::shared_ptr<Mesh> operator()(const std::string& path) const;
    };

    using MeshRef = std::shared_ptr<Mesh>;
}
