#pragma once

#include <meatengine/td/Mesh.hpp>
#include <meatengine/td/cmp/MeshRenderer.hpp>

namespace me::td {
    void renderer_set_mesh(cmp::MeshRenderer& renderer, MeshRef mesh);
    void renderer_set_mesh_path(cmp::MeshRenderer& renderer, const std::string& path);
    void renderer_clear_mesh(cmp::MeshRenderer& renderer);
    bool renderer_has_mesh(const cmp::MeshRenderer& renderer);
}
