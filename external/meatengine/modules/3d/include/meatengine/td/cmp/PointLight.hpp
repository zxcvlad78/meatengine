#pragma once

#include <glm/glm.hpp>

namespace me::td::cmp {
    // Point light. Position comes from the entity's GlobalTransform3D (or
    // Transform3D). Lights with range <= 0 are skipped.
    struct PointLight {
        glm::vec3 color{1.f, 1.f, 1.f};
        float intensity = 1.f;
        float range = 10.f;
    };
}
