#pragma once

#include <glm/glm.hpp>

namespace me::td::cmp {
    // Spot light. Position and world direction come from the entity
    // transform, like DirectionalLight. Angles are degrees, outer >= inner.
    struct SpotLight {
        glm::vec3 color{1.f, 1.f, 1.f};
        float intensity = 1.f;
        float range = 15.f;
        float inner_angle_deg = 12.5f;
        float outer_angle_deg = 20.f;
        glm::vec3 direction{0.f, -1.f, 0.f}; // local forward, normalized in the pass
    };
}
