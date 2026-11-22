#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace me::td {
    glm::quat look_at_rotation(const glm::vec3& position,
                               const glm::vec3& target,
                               const glm::vec3& up = glm::vec3(0.f, 1.f, 0.f));

    glm::vec3 forward(const glm::mat4& world);
    glm::vec3 right(const glm::mat4& world);
    glm::vec3 up(const glm::mat4& world);

    glm::vec3 position_of(const glm::mat4& world);

    glm::mat4 compose_transform(const glm::vec3& position,
                                const glm::quat& rotation,
                                const glm::vec3& scale);

    glm::quat from_to_rotation(const glm::vec3& from, const glm::vec3& to);
    glm::quat axis_angle(float angle_deg, const glm::vec3& axis);
    void rotate_by(glm::quat& rotation, float angle_deg, const glm::vec3& local_axis);
}
