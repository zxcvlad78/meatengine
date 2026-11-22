#include <stdexcept>
#include <string>

#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <meatengine/app/MainLoop.hpp>
#include <meatengine/assets/ResourceLoader.hpp>
#include <meatengine/scripting/LuaBinder.hpp>

#include <meatengine/td/CameraOps.hpp>
#include <meatengine/td/Math.hpp>
#include <meatengine/td/Mesh.hpp>
#include <meatengine/td/Primitives.hpp>
#include <meatengine/td/RenderState.hpp>
#include <meatengine/td/RendererOps.hpp>
#include <meatengine/td/MaterialOps.hpp>
#include <meatengine/td/cmp/Camera3D.hpp>
#include <meatengine/td/cmp/GlobalTransform3D.hpp>
#include <meatengine/td/cmp/Material3D.hpp>
#include <meatengine/td/cmp/MeshRenderer.hpp>
#include <meatengine/td/cmp/Transform3D.hpp>

// The global `td` table: mesh creation, scene settings, 3D helpers.
// Usage from Lua:
//   local e = reg:create()
//   reg:add_Transform3D(e)
//   local r = reg:add_MeshRenderer(e)
//   td.set_mesh(r, td.cube(1))           -- or: td.set_mesh(r, "res/models/x.obj")
//   local m = reg:add_Material3D(e)
//   m.albedo = td.vec3(1, 0.3, 0.2)
//   td.set_texture(m, "res/textures/brick.png")

static me::td::Scene3D& scene() {
    return me::td::ensure_scene3d(me::MainLoop::get_registry());
}

static void bind(sol::state& lua) {
    auto td = lua.create_table();
    using namespace me::td;

    // --- primitives ----------------------------------------------------
    td.set_function("cube", sol::overload(
        [](float size) { return primitives::cube(glm::vec3(size)); },
        [](float w, float h, float d) { return primitives::cube(glm::vec3(w, h, d)); },
        [](const glm::vec3& size) { return primitives::cube(size); }
    ));
    td.set_function("sphere", sol::overload(
        [](float radius) { return primitives::sphere(radius); },
        [](float radius, int segments) { return primitives::sphere(radius, segments); },
        [](float radius, int segments, int rings) { return primitives::sphere(radius, segments, rings); }
    ));
    td.set_function("plane", sol::overload(
        []() { return primitives::plane(); },
        [](float width, float depth) { return primitives::plane(width, depth); },
        [](float width, float depth, int subdivisions) { return primitives::plane(width, depth, subdivisions); }
    ));
    td.set_function("cylinder", sol::overload(
        [](float radius, float height) { return primitives::cylinder(radius, height); },
        [](float radius, float height, int segments) { return primitives::cylinder(radius, height, segments); }
    ));
    td.set_function("cone", sol::overload(
        [](float radius, float height) { return primitives::cone(radius, height); },
        [](float radius, float height, int segments) { return primitives::cone(radius, height, segments); }
    ));
    td.set_function("capsule", sol::overload(
        [](float radius, float height) { return primitives::capsule(radius, height); },
        [](float radius, float height, int segments) { return primitives::capsule(radius, height, segments); }
    ));
    td.set_function("torus", [](float major, float minor) { return primitives::torus(major, minor); });
    td.set_function("quad", []() { return primitives::quad(); });
    td.set_function("grid", sol::overload(
        []() { return primitives::grid(); },
        [](float size) { return primitives::grid(size); },
        [](float size, int divisions) { return primitives::grid(size, divisions); }
    ));

    // --- meshes ----------------------------------------------------------
    td.set_function("load_mesh", [](const std::string& path) -> MeshRef {
        auto resource = me::ResourceLoader::load<Mesh>(path);
        if (!resource) return nullptr;
        return resource.handle();
    });
    td.set_function("set_mesh", sol::overload(
        [](cmp::MeshRenderer& renderer, MeshRef mesh) { renderer_set_mesh(renderer, std::move(mesh)); },
        [](cmp::MeshRenderer& renderer, const std::string& path) { renderer_set_mesh_path(renderer, path); }
    ));

    // --- materials ---------------------------------------------------------
    td.set_function("set_texture", [](cmp::Material3D& material, const std::string& path) {
        auto texture = me::ResourceLoader::load<me::Texture>(path);
        if (!texture) throw std::runtime_error("td.set_texture: cannot load '" + path + "'");
        material_set_texture(material, texture);
    });

    // --- orientation -------------------------------------------------------
    td.set_function("look_at", sol::overload(
        [](cmp::Transform3D& t, const glm::vec3& target) {
            t.rotation = look_at_rotation(t.position, target);
        },
        [](cmp::Transform3D& t, const glm::vec3& target, const glm::vec3& up) {
            t.rotation = look_at_rotation(t.position, target, up);
        }
    ));
    td.set_function("rotate", [](cmp::Transform3D& t, float deg, const glm::vec3& axis) {
        rotate_by(t.rotation, deg, axis);
    });
    td.set_function("axis_angle", [](float deg, const glm::vec3& axis) {
        return axis_angle(deg, axis);
    });

    // --- cameras -----------------------------------------------------------
    td.set_function("make_current", [](entt::registry& reg, entt::entity e) {
        if (auto* camera = reg.try_get<cmp::Camera3D>(e))
            camera3d_make_current(reg, *camera);
    });
    td.set_function("get_current_camera", [](entt::registry& reg) -> cmp::Camera3D* {
        return camera3d_get_current(reg);
    });

    // --- scene settings ------------------------------------------------------
    td.set_function("set_clear_color", sol::overload(
        [](float r, float g, float b) { scene().clear_color_value = {r, g, b}; },
        [](const glm::vec3& c) { scene().clear_color_value = c; }
    ));
    td.set_function("enable_fog", [](bool enabled) { scene().fog_enabled = enabled; });
    td.set_function("set_fog", [](const glm::vec3& color, float start, float end) {
        auto& s = scene();
        s.fog_enabled = true;
        s.fog_color = color;
        s.fog_start = start;
        s.fog_end = end;
    });
    td.set_function("set_wireframe", [](bool enabled) { scene().wireframe = enabled; });

    // --- vector math helpers --------------------------------------------------
    td.set_function("vec3", [](float x, float y, float z) { return glm::vec3(x, y, z); });
    td.set_function("length", [](const glm::vec3& v) { return glm::length(v); });
    td.set_function("normalize", [](const glm::vec3& v) { return glm::normalize(v); });
    td.set_function("dot", [](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); });
    td.set_function("cross", [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); });
    td.set_function("distance", [](const glm::vec3& a, const glm::vec3& b) { return glm::distance(a, b); });
    td.set_function("lerp", [](const glm::vec3& a, const glm::vec3& b, float t) { return a + (b - a) * t; });
    td.set_function("forward", [](const cmp::GlobalTransform3D& g) { return forward(g.matrix); });
    td.set_function("right", [](const cmp::GlobalTransform3D& g) { return right(g.matrix); });
    td.set_function("up", [](const cmp::GlobalTransform3D& g) { return up(g.matrix); });
    td.set_function("position_of", [](const cmp::GlobalTransform3D& g) { return position_of(g.matrix); });

    lua["td"] = td;
}

ME_LUA_BINDING(
    "engine:3d:api",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)
