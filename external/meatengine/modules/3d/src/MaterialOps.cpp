#include <meatengine/td/MaterialOps.hpp>

#include <meatengine/assets/ResourceLoader.hpp>

namespace me::td {

    void material_set_texture(cmp::Material3D& material, entt::resource<me::Texture> texture) {
        material.texture = std::move(texture);
    }

    void material_set_texture_from(cmp::Material3D& material, me::Texture& texture) {
        auto handle = me::ResourceLoader::find_handle(&texture);
        if (!handle) {
            throw std::runtime_error(
                "material_set_texture_from: Texture not from ResourceLoader");
        }
        material.texture = entt::resource<me::Texture>{handle};
    }

    void material_clear_texture(cmp::Material3D& material) {
        material.texture = {};
    }

    bool material_has_texture(const cmp::Material3D& material) {
        return static_cast<bool>(material.texture);
    }
}
