#pragma once

#include <glm/glm.hpp>

namespace me::td::cmp {
    // Directional light. World direction = GlobalTransform3D * `direction`
    // (rotating the light entity rotates the light); falls back to `direction`
    // alone when the entity has no transform.
    struct DirectionalLight {
        glm::vec3 color{1.f, 1.f, 1.f};
        float intensity = 1.f;
        glm::vec3 direction{0.f, -1.f, 0.f}; // local forward, normalized in the pass
    };
}
