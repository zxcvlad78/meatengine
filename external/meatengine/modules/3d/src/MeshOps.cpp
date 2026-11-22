#include <meatengine/td/MeshOps.hpp>

#include <limits>

namespace me::td {

    bool mesh_empty(const Mesh& mesh) {
        return mesh.vertices.empty() || mesh.indices.empty();
    }

    void mesh_set_data(Mesh& mesh, std::vector<MeshVertex> vertices, std::vector<uint32_t> indices) {
        mesh.vertices = std::move(vertices);
        mesh.indices = std::move(indices);
        mesh.has_aabb = false;
        mesh_recompute_aabb(mesh);
    }

    void mesh_recompute_aabb(Mesh& mesh) {
        mesh.aabb_min = glm::vec3(std::numeric_limits<float>::max());
        mesh.aabb_max = glm::vec3(std::numeric_limits<float>::lowest());
        for (const auto& v : mesh.vertices) {
            mesh.aabb_min = glm::min(mesh.aabb_min, v.position);
            mesh.aabb_max = glm::max(mesh.aabb_max, v.position);
        }
        mesh.has_aabb = !mesh.vertices.empty();
    }
}
