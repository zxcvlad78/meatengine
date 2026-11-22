#pragma once

#include <glm/glm.hpp>

namespace me::td::cmp {
    // World-space transform, computed every frame by the "engine:3d:transform"
    // system. Do not set manually unless the entity has no Transform3D.
    struct GlobalTransform3D {
        glm::mat4 matrix{1.f};
    };
}
