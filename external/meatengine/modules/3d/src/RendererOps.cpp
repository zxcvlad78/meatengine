#include <meatengine/td/RendererOps.hpp>

#include <meatengine/assets/ResourceLoader.hpp>

namespace me::td {

    void renderer_set_mesh(cmp::MeshRenderer& renderer, MeshRef mesh) {
        renderer.mesh = entt::resource<Mesh>{std::move(mesh)};
    }

    void renderer_set_mesh_path(cmp::MeshRenderer& renderer, const std::string& path) {
        renderer.mesh = me::ResourceLoader::load<Mesh>(path);
    }

    void renderer_clear_mesh(cmp::MeshRenderer& renderer) {
        renderer.mesh = {};
    }

    bool renderer_has_mesh(const cmp::MeshRenderer& renderer) {
        return static_cast<bool>(renderer.mesh);
    }
}
