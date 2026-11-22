#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace me::td::cmp {
    // Perspective/orthographic 3D camera. Exactly one camera should have
    // current == true; it defines what the "engine:3d:render" pass draws.
    // view/projection are filled by the render pass (read-only data).
    struct Camera3D {
        float fov_deg = 60.f;    // vertical FOV (perspective only)
        float near_z = 0.1f;
        float far_z = 1000.f;
        bool perspective = true;
        float ortho_size = 10.f; // half-height of the ortho frustum
        bool current = true;

        glm::mat4 view{1.f};
        glm::mat4 projection{1.f};
    };
}
