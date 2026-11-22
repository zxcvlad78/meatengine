#pragma once

#include <entt/resource/resource.hpp>
#include <glm/glm.hpp>

#include <meatengine/assets/Resources.hpp>

namespace me::td::cmp {
    // Surface description for a MeshRenderer. Optional: if the entity has no
    // Material3D, a default white material is used by the render pass.
    struct Material3D {
        glm::vec3 albedo{1.f, 1.f, 1.f};

        // Optional albedo texture (me::Texture from the assets module, so any
        // sf::Texture-loaded image works). Leave empty for a plain color.
        entt::resource<me::Texture> texture{};

        float emissive = 0.f;   // self-glow strength, multiplies albedo
        float shininess = 32.f; // Blinn-Phong exponent
        float specular = 0.5f;  // specular strength

        float opacity = 1.f;    // < 1 -> drawn in the transparent queue

        bool unlit = false;     // skip lighting entirely (albedo/emissive only)
        bool two_sided = false; // disable back-face culling for this mesh
    };
}
