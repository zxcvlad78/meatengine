#include <meatengine/td/CameraOps.hpp>

namespace me::td {

    cmp::Camera3D* camera3d_get_current(entt::registry& reg) {
        for (auto [entity, camera] : reg.view<cmp::Camera3D>().each()) {
            if (camera.current) return &camera;
        }
        return nullptr;
    }

    void camera3d_make_current(entt::registry& reg, cmp::Camera3D& camera) {
        if (cmp::Camera3D* prev = camera3d_get_current(reg))
            prev->current = false;
        camera.current = true;
    }
}
