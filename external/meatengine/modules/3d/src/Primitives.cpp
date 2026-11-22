#include <meatengine/td/Primitives.hpp>

#include <meatengine/td/MeshOps.hpp>

#include <algorithm>
#include <cmath>

#include <glm/gtc/constants.hpp>

namespace me::td::primitives {

    namespace {

        std::shared_ptr<Mesh> finish(std::vector<MeshVertex> v, std::vector<uint32_t> i) {
            auto mesh = std::make_shared<Mesh>();
            mesh_set_data(*mesh, std::move(v), std::move(i));
            return mesh;
        }

        // Appends a quad as two CCW triangles (per-face normal).
        void quad_face(std::vector<MeshVertex>& v, std::vector<uint32_t>& idx,
                       const glm::vec3& a, const glm::vec3& b,
                       const glm::vec3& c, const glm::vec3& d,
                       const glm::vec3& n, const glm::vec2& uv_a = {0, 0},
                       const glm::vec2& uv_b = {1, 0},
                       const glm::vec2& uv_c = {1, 1},
                       const glm::vec2& uv_d = {0, 1}) {
            uint32_t base = static_cast<uint32_t>(v.size());
            v.push_back({a, n, uv_a});
            v.push_back({b, n, uv_b});
            v.push_back({c, n, uv_c});
            v.push_back({d, n, uv_d});
            idx.insert(idx.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
        }
    }

    MeshRef cube(const glm::vec3& size) {
        glm::vec3 h = size * 0.5f;

        std::vector<MeshVertex> v;
        std::vector<uint32_t> idx;
        v.reserve(24);
        idx.reserve(36);

        // -X / +X
        quad_face(v, idx, {-h.x,-h.y,-h.z}, {-h.x,-h.y, h.z}, {-h.x, h.y, h.z}, {-h.x, h.y,-h.z}, {-1, 0, 0});
        quad_face(v, idx, { h.x,-h.y,-h.z}, { h.x, h.y,-h.z}, { h.x, h.y, h.z}, { h.x,-h.y, h.z}, { 1, 0, 0});
        // -Y / +Y
        quad_face(v, idx, {-h.x,-h.y,-h.z}, { h.x,-h.y,-h.z}, { h.x,-h.y, h.z}, {-h.x,-h.y, h.z}, { 0,-1, 0});
        quad_face(v, idx, {-h.x, h.y,-h.z}, {-h.x, h.y, h.z}, { h.x, h.y, h.z}, { h.x, h.y,-h.z}, { 0, 1, 0});
        // -Z / +Z
        quad_face(v, idx, {-h.x,-h.y, h.z}, { h.x,-h.y, h.z}, { h.x, h.y, h.z}, {-h.x, h.y, h.z}, { 0, 0, 1});
        quad_face(v, idx, {-h.x,-h.y,-h.z}, {-h.x, h.y,-h.z}, { h.x, h.y,-h.z}, { h.x,-h.y,-h.z}, { 0, 0,-1});

        return finish(std::move(v), std::move(idx));
    }

    MeshRef sphere(float radius, int segments, int rings) {
        segments = std::clamp(segments, 3, 256);
        rings = std::clamp(rings, 2, 256);

        std::vector<MeshVertex> v;
        std::vector<uint32_t> idx;
        v.reserve(static_cast<size_t>((rings + 1) * (segments + 1)));

        for (int r = 0; r <= rings; ++r) {
            float phi = glm::pi<float>() * static_cast<float>(r) / static_cast<float>(rings);
            float sy = std::cos(phi), sr = std::sin(phi);
            for (int s = 0; s <= segments; ++s) {
                float theta = glm::two_pi<float>() * static_cast<float>(s) / static_cast<float>(segments);
                glm::vec3 n{sr * std::cos(theta), sy, sr * std::sin(theta)};
                v.push_back({n * radius, n, {static_cast<float>(s) / segments,
                                             static_cast<float>(r) / rings}});
            }
        }

        auto at = [cols = segments + 1](int r, int s) {
            return static_cast<uint32_t>(r * cols + s);
        };

        for (int r = 0; r < rings; ++r) {
            for (int s = 0; s < segments; ++s) {
                uint32_t a = at(r, s), b = at(r + 1, s), c = at(r + 1, s + 1), d = at(r, s + 1);
                if (r != 0) idx.insert(idx.end(), {a, b, d});
                if (r != rings - 1) idx.insert(idx.end(), {b, c, d});
            }
        }
        return finish(std::move(v), std::move(idx));
    }

    MeshRef plane(float width, float depth, int subdivisions) {
        subdivisions = std::clamp(subdivisions, 1, 512);
        std::vector<MeshVertex> v;
        std::vector<uint32_t> idx;
        v.reserve(static_cast<size_t>((subdivisions + 1) * (subdivisions + 1)));

        const glm::vec3 n(0.f, 1.f, 0.f);
        for (int z = 0; z <= subdivisions; ++z) {
            for (int x = 0; x <= subdivisions; ++x) {
                float fx = static_cast<float>(x) / subdivisions;
                float fz = static_cast<float>(z) / subdivisions;
                v.push_back({{(fx - 0.5f) * width, 0.f, (fz - 0.5f) * depth},
                             n, {fx, fz}});
            }
        }

        auto at = [cols = subdivisions + 1](int z, int x) {
            return static_cast<uint32_t>(z * cols + x);
        };
        for (int z = 0; z < subdivisions; ++z) {
            for (int x = 0; x < subdivisions; ++x) {
                uint32_t a = at(z, x), b = at(z + 1, x), c = at(z + 1, x + 1), d = at(z, x + 1);
                idx.insert(idx.end(), {a, c, b, a, d, c});
            }
        }
        return finish(std::move(v), std::move(idx));
    }

    MeshRef cylinder(float radius, float height, int segments) {
        segments = std::clamp(segments, 3, 256);
        float hy = height * 0.5f;

        std::vector<MeshVertex> v;
        std::vector<uint32_t> idx;

        // Side wall.
        for (int s = 0; s <= segments; ++s) {
            float t = glm::two_pi<float>() * static_cast<float>(s) / segments;
            glm::vec3 n{std::cos(t), 0.f, std::sin(t)};
            glm::vec2 uv{static_cast<float>(s) / segments, 1.f};
            v.push_back({n * radius + glm::vec3(0, hy, 0), n, uv});
            v.push_back({n * radius - glm::vec3(0, hy, 0), n, {uv.x, 0.f}});
        }
        for (int s = 0; s < segments; ++s) {
            uint32_t base = static_cast<uint32_t>(s * 2);
            idx.insert(idx.end(), {base, base + 2, base + 1, base + 1, base + 2, base + 3});
        }

        // Caps (shared center vertex per cap).
        uint32_t top_center = static_cast<uint32_t>(v.size());
        v.push_back({{0, hy, 0}, {0, 1, 0}, {0.5f, 0.5f}});
        uint32_t bot_center = static_cast<uint32_t>(v.size());
        v.push_back({{0, -hy, 0}, {0, -1, 0}, {0.5f, 0.5f}});

        for (int s = 0; s <= segments; ++s) {
            float t = glm::two_pi<float>() * static_cast<float>(s) / segments;
            float cs = std::cos(t), sn = std::sin(t);
            v.push_back({{radius * cs, hy, radius * sn}, {0, 1, 0}, {cs * 0.5f + 0.5f, sn * 0.5f + 0.5f}});
            v.push_back({{radius * cs, -hy, radius * sn}, {0, -1, 0}, {cs * 0.5f + 0.5f, sn * 0.5f + 0.5f}});
        }
        uint32_t ring_start = top_center + 2;
        for (int s = 0; s < segments; ++s) {
            uint32_t a = ring_start + static_cast<uint32_t>(s * 2);
            idx.insert(idx.end(), {top_center, a + 2, a});         // top cap (CCW from above)
            idx.insert(idx.end(), {bot_center, a + 1, a + 3});     // bottom cap
        }

        return finish(std::move(v), std::move(idx));
    }

    MeshRef cone(float radius, float height, int segments) {
        segments = std::clamp(segments, 3, 256);
        float hy = height * 0.5f;

        std::vector<MeshVertex> v;
        std::vector<uint32_t> idx;

        // Side wall (per-face normals for a crisp look).
        glm::vec3 apex(0.f, hy, 0.f);
        for (int s = 0; s < segments; ++s) {
            float t0 = glm::two_pi<float>() * static_cast<float>(s) / segments;
            float t1 = glm::two_pi<float>() * static_cast<float>(s + 1) / segments;
            glm::vec3 p0{radius * std::cos(t0), -hy, radius * std::sin(t0)};
            glm::vec3 p1{radius * std::cos(t1), -hy, radius * std::sin(t1)};

            glm::vec3 mid = glm::normalize((p0 + p1) * 0.5f + glm::vec3(0, height, 0));
            uint32_t base = static_cast<uint32_t>(v.size());
            v.push_back({p0, mid, {static_cast<float>(s) / segments, 0.f}});
            v.push_back({p1, mid, {static_cast<float>(s + 1) / segments, 0.f}});
            v.push_back({apex, mid, {static_cast<float>(s + 0.5f) / segments, 1.f}});
            idx.insert(idx.end(), {base, base + 2, base + 1});
        }

        // Base cap.
        uint32_t center = static_cast<uint32_t>(v.size());
        v.push_back({{0, -hy, 0}, {0, -1, 0}, {0.5f, 0.5f}});
        for (int s = 0; s <= segments; ++s) {
            float t = glm::two_pi<float>() * static_cast<float>(s) / segments;
            float cs = std::cos(t), sn = std::sin(t);
            v.push_back({{radius * cs, -hy, radius * sn}, {0, -1, 0}, {cs * 0.5f + 0.5f, sn * 0.5f + 0.5f}});
        }
        uint32_t ring_start = center + 1;
        for (int s = 0; s < segments; ++s) {
            idx.insert(idx.end(), {center, ring_start + static_cast<uint32_t>(s),
                                   ring_start + static_cast<uint32_t>(s + 1)});
        }

        return finish(std::move(v), std::move(idx));
    }

    MeshRef capsule(float radius, float height, int segments, int cap_rings) {
        segments = std::clamp(segments, 3, 256);
        cap_rings = std::clamp(cap_rings, 2, 64);
        // height = distance between the two hemisphere centers.
        float half = height * 0.5f;

        std::vector<MeshVertex> v;
        std::vector<uint32_t> idx;

        // Middle cylinder wall.
        for (int s = 0; s <= segments; ++s) {
            float t = glm::two_pi<float>() * static_cast<float>(s) / segments;
            glm::vec3 n{std::cos(t), 0.f, std::sin(t)};
            v.push_back({n * radius + glm::vec3(0, half, 0), n, {static_cast<float>(s) / segments, 1.f}});
            v.push_back({n * radius - glm::vec3(0, half, 0), n, {static_cast<float>(s) / segments, 0.f}});
        }
        for (int s = 0; s < segments; ++s) {
            uint32_t base = static_cast<uint32_t>(s * 2);
            idx.insert(idx.end(), {base, base + 2, base + 1, base + 1, base + 2, base + 3});
        }

        // Hemisphere ring mesh (from the equator to the pole).
        auto push_hemisphere = [&](float center_y, bool top) {
            uint32_t ring_start = static_cast<uint32_t>(v.size());
            for (int r = 0; r <= cap_rings; ++r) {
                float phi = (glm::pi<float>() * 0.5f) * static_cast<float>(r) / cap_rings;
                float sy = std::sin(phi), sr = std::cos(phi); // from equator to pole
                if (!top) sy = -sy;
                for (int s = 0; s <= segments; ++s) {
                    float t = glm::two_pi<float>() * static_cast<float>(s) / segments;
                    float cs = std::cos(t), sn = std::sin(t);
                    glm::vec3 n{sr * cs, sy, sr * sn};
                    v.push_back({n * radius + glm::vec3(0, center_y, 0), n,
                                 {static_cast<float>(s) / segments,
                                  top ? 0.5f + 0.5f * (1.f - sy) : 0.5f * (1.f + sy)}});
                }
            }
            return ring_start;
        };

        // Top hemisphere: stitched to the cylinder's top ring (even wall vertices).
        uint32_t top_start = push_hemisphere(half, true);
        for (int s = 0; s < segments; ++s) {
            uint32_t w0 = static_cast<uint32_t>(s * 2);
            uint32_t t0 = top_start + static_cast<uint32_t>(s);
            idx.insert(idx.end(), {w0, t0 + 1, t0, w0, w0 + 2, t0 + 1});
        }
        // Top pole triangles.
        {
            uint32_t pole_row = top_start + static_cast<uint32_t>(cap_rings * (segments + 1));
            uint32_t pole = static_cast<uint32_t>(v.size());
            v.push_back({{0, half + radius, 0}, {0, 1, 0}, {0.5f, 1.f}});
            for (int s = 0; s < segments; ++s) {
                idx.insert(idx.end(), {pole_row + static_cast<uint32_t>(s),
                                       pole, pole_row + static_cast<uint32_t>(s + 1)});
            }
        }

        // Bottom hemisphere (mirrored): stitched to the bottom ring (odd wall vertices).
        uint32_t bot_start = push_hemisphere(-half, false);
        for (int s = 0; s < segments; ++s) {
            uint32_t w1 = 1 + static_cast<uint32_t>(s * 2);
            uint32_t b0 = bot_start + static_cast<uint32_t>(s);
            idx.insert(idx.end(), {w1, b0, b0 + 1, w1, b0 + 1, w1 + 2});
        }
        // Bottom pole triangles.
        {
            uint32_t pole_row = bot_start + static_cast<uint32_t>(cap_rings * (segments + 1));
            uint32_t pole = static_cast<uint32_t>(v.size());
            v.push_back({{0, -half - radius, 0}, {0, -1, 0}, {0.5f, 0.f}});
            for (int s = 0; s < segments; ++s) {
                idx.insert(idx.end(), {pole_row + static_cast<uint32_t>(s),
                                       pole_row + static_cast<uint32_t>(s + 1), pole});
            }
        }

        return finish(std::move(v), std::move(idx));
    }

    MeshRef torus(float major_radius, float minor_radius, int major_segments, int minor_segments) {
        major_segments = std::clamp(major_segments, 3, 256);
        minor_segments = std::clamp(minor_segments, 3, 128);

        std::vector<MeshVertex> v;
        std::vector<uint32_t> idx;

        for (int i = 0; i <= major_segments; ++i) {
            float u = glm::two_pi<float>() * static_cast<float>(i) / major_segments;
            float cu = std::cos(u), su = std::sin(u);
            for (int j = 0; j <= minor_segments; ++j) {
                float w = glm::two_pi<float>() * static_cast<float>(j) / minor_segments;
                float cw = std::cos(w), sw = std::sin(w);
                glm::vec3 n{cw * cu, sw, cw * su};
                glm::vec3 p{(major_radius + minor_radius * cw) * cu,
                            minor_radius * sw,
                            (major_radius + minor_radius * cw) * su};
                v.push_back({p, n, {static_cast<float>(i) / major_segments,
                                    static_cast<float>(j) / minor_segments}});
            }
        }

        auto at = [cols = minor_segments + 1](int i, int j) {
            return static_cast<uint32_t>(i * cols + j);
        };
        for (int i = 0; i < major_segments; ++i) {
            for (int j = 0; j < minor_segments; ++j) {
                uint32_t a = at(i, j), b = at(i + 1, j), c = at(i + 1, j + 1), d = at(i, j + 1);
                idx.insert(idx.end(), {a, d, b, b, d, c});
            }
        }
        return finish(std::move(v), std::move(idx));
    }

    MeshRef quad() {
        std::vector<MeshVertex> v;
        std::vector<uint32_t> idx;
        quad_face(v, idx, {-0.5f,-0.5f, 0.f}, {0.5f,-0.5f, 0.f}, {0.5f, 0.5f, 0.f}, {-0.5f, 0.5f, 0.f},
                  {0.f, 0.f, 1.f});
        return finish(std::move(v), std::move(idx));
    }

    MeshRef grid(float size, int divisions) {
        divisions = std::clamp(divisions, 1, 512);
        std::vector<MeshVertex> v;
        std::vector<uint32_t> idx;
        float half = size * 0.5f;

        for (int i = 0; i <= divisions; ++i) {
            float t = -half + size * static_cast<float>(i) / divisions;

            uint32_t base = static_cast<uint32_t>(v.size());
            v.push_back({{t, 0.f, -half}, {0, 1, 0}, {0, 0}});
            v.push_back({{t, 0.f,  half}, {0, 1, 0}, {0, 0}});
            idx.push_back(base);
            idx.push_back(base + 1);

            base = static_cast<uint32_t>(v.size());
            v.push_back({{-half, 0.f, t}, {0, 1, 0}, {0, 0}});
            v.push_back({{ half, 0.f, t}, {0, 1, 0}, {0, 0}});
            idx.push_back(base);
            idx.push_back(base + 1);
        }
        return finish(std::move(v), std::move(idx));
    }
}
