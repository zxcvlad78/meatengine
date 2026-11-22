#pragma once

#include <string>

#include <meatengine/td/Mesh.hpp>

namespace me::td {
    MeshRef load_mesh_obj(const std::string& path);
}
