#pragma once

#include <entt/resource/resource.hpp>

#include <meatengine/assets/Resources.hpp>
#include <meatengine/td/cmp/Material3D.hpp>

namespace me::td {
    void material_set_texture(cmp::Material3D& material, entt::resource<me::Texture> texture);
    void material_set_texture_from(cmp::Material3D& material, me::Texture& texture);
    void material_clear_texture(cmp::Material3D& material);
    bool material_has_texture(const cmp::Material3D& material);
}
