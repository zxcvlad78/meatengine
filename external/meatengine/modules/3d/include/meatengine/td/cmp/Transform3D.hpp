#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace me::td::cmp {
    // Local (parent-relative) 3D transform.
    // GlobalTransform3D is computed from this each frame by the
    // "engine:3d:transform" system (respecting hierarchy Parent/Children).
    struct Transform3D {
        glm::vec3 position{0.f, 0.f, 0.f};
        glm::quat rotation{1.f, 0.f, 0.f, 0.f}; // identity
        glm::vec3 scale{1.f, 1.f, 1.f};
    };
}
