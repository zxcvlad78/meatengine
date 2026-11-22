#pragma once

#include <glad/glad.h>

#include <meatengine/td/Mesh.hpp>

namespace me::td {
    void mesh_upload(const Mesh& mesh);
    void mesh_draw(const Mesh& mesh, GLenum mode = GL_TRIANGLES);
    void mesh_free_gpu(Mesh& mesh);
}
