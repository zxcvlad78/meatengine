#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace me::td {
    struct Scene3D {
        bool clear_color = true;
        bool clear_depth = true;
        glm::vec3 clear_color_value{0.f, 0.f, 0.f};

        bool fog_enabled = false;
        float fog_start = 20.f;
        float fog_end = 100.f;
        glm::vec3 fog_color{0.f, 0.f, 0.f};

        bool wireframe = false;

        glm::mat4 view{1.f};
        glm::mat4 projection{1.f};
        glm::vec3 view_pos{0.f};
        bool has_camera = false;
    };

    Scene3D& ensure_scene3d(entt::registry& reg);
}
