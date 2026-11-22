#include <meatengine/td/Math.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace me::td {

    glm::quat look_at_rotation(const glm::vec3& position,
                               const glm::vec3& target,
                               const glm::vec3& up) {
        glm::vec3 dir = target - position;
        if (glm::dot(dir, dir) < 1e-12f) return glm::quat(1.f, 0.f, 0.f, 0.f);
        dir = glm::normalize(dir);

        // glm::quatLookAt orients -Z along `dir` (camera convention).
        return glm::quatLookAt(dir, up);
    }

    glm::vec3 forward(const glm::mat4& world) {
        return -glm::vec3(world[2]);
    }

    glm::vec3 right(const glm::mat4& world) {
        return glm::vec3(world[0]);
    }

    glm::vec3 up(const glm::mat4& world) {
        return glm::vec3(world[1]);
    }

    glm::vec3 position_of(const glm::mat4& world) {
        return glm::vec3(world[3]);
    }

    glm::mat4 compose_transform(const glm::vec3& position,
                                const glm::quat& rotation,
                                const glm::vec3& scale) {
        glm::mat4 model(1.f);
        model = glm::translate(model, position);
        model *= glm::mat4_cast(rotation);
        model = glm::scale(model, scale);
        return model;
    }

    glm::quat from_to_rotation(const glm::vec3& from, const glm::vec3& to) {
        glm::vec3 f = glm::normalize(from);
        glm::vec3 t = glm::normalize(to);
        float d = glm::dot(f, t);
        if (d > 1.f - 1e-6f) return glm::quat(1.f, 0.f, 0.f, 0.f);
        if (d < -1.f + 1e-6f) {
            // Opposite direction: rotate 180 degrees around any perpendicular axis.
            glm::vec3 axis = glm::cross(f, glm::vec3(1.f, 0.f, 0.f));
            if (glm::dot(axis, axis) < 1e-6f)
                axis = glm::cross(f, glm::vec3(0.f, 1.f, 0.f));
            return glm::angleAxis(glm::pi<float>(), glm::normalize(axis));
        }
        // Shortest-arc quaternion: q = normalize([1+d, cross(f, t)]).
        glm::vec3 c = glm::cross(f, t);
        return glm::normalize(glm::quat(1.f + d, c.x, c.y, c.z));
    }

    glm::quat axis_angle(float angle_deg, const glm::vec3& axis) {
        glm::vec3 a = glm::normalize(axis);
        if (glm::dot(a, a) < 1e-12f) return glm::quat(1.f, 0.f, 0.f, 0.f);
        return glm::angleAxis(glm::radians(angle_deg), a);
    }

    void rotate_by(glm::quat& rotation, float angle_deg, const glm::vec3& local_axis) {
        rotation = rotation * axis_angle(angle_deg, local_axis);
    }
}
