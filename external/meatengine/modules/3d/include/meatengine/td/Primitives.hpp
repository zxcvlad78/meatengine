#pragma once

#include <glm/glm.hpp>

#include <meatengine/td/Mesh.hpp>

namespace me::td::primitives {
    MeshRef cube(const glm::vec3& size = glm::vec3(1.f));
    MeshRef sphere(float radius = 0.5f, int segments = 24, int rings = 12);
    MeshRef plane(float width = 1.f, float depth = 1.f, int subdivisions = 1); // faces +Y
    MeshRef cylinder(float radius = 0.5f, float height = 1.f, int segments = 24);
    MeshRef cone(float radius = 0.5f, float height = 1.f, int segments = 24);
    MeshRef capsule(float radius = 0.25f, float height = 1.f, int segments = 24, int cap_rings = 8);
    MeshRef torus(float major_radius = 0.5f, float minor_radius = 0.2f, int major_segments = 32, int minor_segments = 16);
    MeshRef quad(); // single quad in the XY plane, facing +Z
    MeshRef grid(float size = 10.f, int divisions = 10); // GL_LINES helper grid on the XZ plane
}
