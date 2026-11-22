#include <meatengine/td/MeshIO.hpp>

#include <meatengine/td/MeshOps.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace me::td {

    // ResourceLoader protocol (see Mesh.hpp).
    std::shared_ptr<Mesh> Mesh::operator()(const std::string& path) const {
        return load_mesh_obj(path);
    }

    MeshRef load_mesh_obj(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[td] load_mesh_obj: cannot open '" << path << "'\n";
            return nullptr;
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        std::vector<MeshVertex> out_vertices;
        std::vector<uint32_t> out_indices;
        std::unordered_map<std::string, uint32_t> corner_map;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream ss(line);
            std::string tag;
            ss >> tag;

            if (tag == "v") {
                glm::vec3 p{};
                ss >> p.x >> p.y >> p.z;
                positions.push_back(p);
            } else if (tag == "vn") {
                glm::vec3 n{};
                ss >> n.x >> n.y >> n.z;
                normals.push_back(glm::normalize(n));
            } else if (tag == "vt") {
                glm::vec2 t{};
                ss >> t.x >> t.y;
                uvs.push_back(t);
            } else if (tag == "f") {
                // Collect the raw corner strings of this (possibly polygonal) face.
                std::vector<std::string> corners;
                std::string corner;
                while (ss >> corner) corners.push_back(corner);
                if (corners.size() < 3) continue;

                auto resolve = [&](const std::string& c) -> uint32_t {
                    auto it = corner_map.find(c);
                    if (it != corner_map.end()) return it->second;

                    // corner format: v[ /vt][ /vn], negative values are relative.
                    auto parse_idx = [&](size_t start) {
                        return std::strtol(c.c_str() + start, nullptr, 10);
                    };

                    long vi = parse_idx(0);
                    long ti = 0, ni = 0;
                    size_t slash1 = c.find('/');
                    if (slash1 != std::string::npos) {
                        size_t slash2 = c.find('/', slash1 + 1);
                        if (slash2 != std::string::npos && slash2 > slash1 + 1)
                            ti = parse_idx(slash1 + 1);
                        if (slash2 != std::string::npos && slash2 + 1 < c.size())
                            ni = parse_idx(slash2 + 1);
                    }

                    auto fix = [](long i, size_t count) -> long {
                        if (i > 0) return i - 1;
                        return static_cast<long>(count) + i;
                    };

                    vi = fix(vi, positions.size());
                    ti = fix(ti, uvs.size());
                    ni = fix(ni, normals.size());

                    MeshVertex v{};
                    v.position = (vi >= 0 && vi < static_cast<long>(positions.size()))
                        ? positions[static_cast<size_t>(vi)] : glm::vec3(0.f);
                    v.uv = (ti >= 0 && ti < static_cast<long>(uvs.size()))
                        ? uvs[static_cast<size_t>(ti)] : glm::vec2(0.f);
                    v.normal = (ni >= 0 && ni < static_cast<long>(normals.size()))
                        ? normals[static_cast<size_t>(ni)] : glm::vec3(0.f, 1.f, 0.f);

                    uint32_t index = static_cast<uint32_t>(out_vertices.size());
                    out_vertices.push_back(v);
                    corner_map.emplace(c, index);
                    return index;
                };

                uint32_t first = resolve(corners[0]);
                for (size_t i = 1; i + 1 < corners.size(); ++i) {
                    out_indices.push_back(first);
                    out_indices.push_back(resolve(corners[i]));
                    out_indices.push_back(resolve(corners[i + 1]));
                }
            }
        }

        // Generate smooth-ish normals for corners that had none.
        bool any_missing = false;
        for (const auto& v : out_vertices)
            if (glm::dot(v.normal, v.normal) < 1e-12f) { any_missing = true; break; }

        if (any_missing) {
            std::vector<glm::vec3> accum(out_vertices.size(), glm::vec3(0.f));
            for (size_t i = 0; i + 2 < out_indices.size(); i += 3) {
                auto& a = out_vertices[out_indices[i]];
                auto& b = out_vertices[out_indices[i + 1]];
                auto& c = out_vertices[out_indices[i + 2]];
                glm::vec3 n = glm::normalize(glm::cross(b.position - a.position,
                                                        c.position - a.position));
                accum[out_indices[i]] += n;
                accum[out_indices[i + 1]] += n;
                accum[out_indices[i + 2]] += n;
            }
            for (size_t i = 0; i < out_vertices.size(); ++i) {
                if (glm::dot(out_vertices[i].normal, out_vertices[i].normal) < 1e-12f && glm::dot(accum[i], accum[i]) > 1e-12f)
                    out_vertices[i].normal = glm::normalize(accum[i]);
            }
        }

        auto mesh = std::make_shared<Mesh>();
        mesh_set_data(*mesh, std::move(out_vertices), std::move(out_indices));
        if (mesh_empty(*mesh)) {
            std::cerr << "[td] load_mesh_obj: '" << path << "' produced an empty mesh\n";
            return nullptr;
        }
        return mesh;
    }
}
