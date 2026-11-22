#pragma once

#include <entt/entt.hpp>

#include <meatengine/td/cmp/Camera3D.hpp>

namespace me::td {
    cmp::Camera3D* camera3d_get_current(entt::registry& reg);
    void camera3d_make_current(entt::registry& reg, cmp::Camera3D& camera);
}
