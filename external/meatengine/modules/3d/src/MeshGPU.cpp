#include <meatengine/td/MeshGPU.hpp>

#include <meatengine/td/MeshOps.hpp>

namespace me::td {

    void mesh_free_gpu(Mesh& mesh) {
        if (mesh.vao) {
            glDeleteVertexArrays(1, &mesh.vao);
            glDeleteBuffers(1, &mesh.vbo);
            glDeleteBuffers(1, &mesh.ebo);
            mesh.vao = mesh.vbo = mesh.ebo = 0;
            mesh.index_count = 0;
        }
    }

    void mesh_upload(const Mesh& mesh) {
        if (mesh_empty(mesh) || mesh.vao) return;

        glGenVertexArrays(1, &const_cast<Mesh&>(mesh).vao);
        glGenBuffers(1, &const_cast<Mesh&>(mesh).vbo);
        glGenBuffers(1, &const_cast<Mesh&>(mesh).ebo);

        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(MeshVertex)),
                     mesh.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(uint32_t)),
                     mesh.indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                              reinterpret_cast<void*>(offsetof(MeshVertex, position)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                              reinterpret_cast<void*>(offsetof(MeshVertex, normal)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                              reinterpret_cast<void*>(offsetof(MeshVertex, uv)));

        glBindVertexArray(0);
    }

    void mesh_draw(const Mesh& mesh, GLenum mode) {
        if (mesh_empty(mesh)) return;

        mesh_upload(mesh);
        glBindVertexArray(mesh.vao);
        glDrawElements(mode, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}
