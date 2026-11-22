#pragma once

#include <cstdint>
#include <vector>

#include <meatengine/td/Mesh.hpp>

namespace me::td {
    bool mesh_empty(const Mesh& mesh);
    void mesh_set_data(Mesh& mesh, std::vector<MeshVertex> vertices, std::vector<uint32_t> indices);
    void mesh_recompute_aabb(Mesh& mesh);
}
