#include <meatengine/td/RenderState.hpp>

namespace me::td {

    Scene3D& ensure_scene3d(entt::registry& reg) {
        auto* scene = reg.ctx().find<Scene3D>();
        if (!scene) scene = &reg.ctx().emplace<Scene3D>();
        return *scene;
    }
}
