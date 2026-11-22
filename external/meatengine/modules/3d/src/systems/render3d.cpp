#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <meatengine/assets/Resources.hpp>
#include <meatengine/td/CameraOps.hpp>
#include <meatengine/td/DefaultShader.hpp>
#include <meatengine/td/MeshGPU.hpp>
#include <meatengine/td/Math.hpp>
#include <meatengine/td/RenderState.hpp>
#include <meatengine/td/ShaderOps.hpp>
#include <meatengine/td/cmp/Camera3D.hpp>
#include <meatengine/td/cmp/DirectionalLight.hpp>
#include <meatengine/td/cmp/GlobalTransform3D.hpp>
#include <meatengine/td/cmp/Material3D.hpp>
#include <meatengine/td/cmp/MeshRenderer.hpp>
#include <meatengine/td/cmp/PointLight.hpp>
#include <meatengine/td/cmp/SpotLight.hpp>
#include <meatengine/td/cmp/Transform3D.hpp>

#include <algorithm>
#include <limits>
#include <vector>

namespace {

    using namespace me;
    using namespace me::td;

    // Uniform array limits; must match DefaultShader.cpp.
    constexpr int MAX_DIR_LIGHTS = 4;
    constexpr int MAX_POINT_LIGHTS = 16;
    constexpr int MAX_SPOT_LIGHTS = 8;

    // Per-frame light data, reused across frames (no allocations).
    struct LightFrame {
        int num_dir = 0;
        int num_point = 0;
        int num_spot = 0;

        glm::vec3 dir_dir[MAX_DIR_LIGHTS]{};
        glm::vec3 dir_color[MAX_DIR_LIGHTS]{};

        glm::vec3 point_pos[MAX_POINT_LIGHTS]{};
        glm::vec3 point_color[MAX_POINT_LIGHTS]{};
        float point_range[MAX_POINT_LIGHTS]{};

        glm::vec3 spot_pos[MAX_SPOT_LIGHTS]{};
        glm::vec3 spot_color[MAX_SPOT_LIGHTS]{};
        glm::vec3 spot_dir[MAX_SPOT_LIGHTS]{};
        float spot_range[MAX_SPOT_LIGHTS]{};
        float spot_inner[MAX_SPOT_LIGHTS]{};
        float spot_outer[MAX_SPOT_LIGHTS]{};

        void clear() { num_dir = num_point = num_spot = 0; }
    };

    // Cached uniform locations of the built-in forward shader layout.
    // Custom shaders fall back to the generic uniform_set_* path.
    struct FastUniforms {
        GLint model = -1;
        GLint normal_mat = -1;
        GLint view = -1;
        GLint proj = -1;
        GLint view_pos = -1;

        GLint albedo = -1;
        GLint emissive = -1;
        GLint shininess = -1;
        GLint specular = -1;
        GLint opacity = -1;
        GLint unlit = -1;

        GLint fog_enabled = -1;
        GLint fog_color = -1;
        GLint fog_start = -1;
        GLint fog_end = -1;

        GLint texture = -1;
        GLint has_texture = -1;

        GLint num_dir = -1;
        GLint dir_dir = -1;
        GLint dir_color = -1;

        GLint num_point = -1;
        GLint point_pos = -1;
        GLint point_color = -1;
        GLint point_range = -1;

        GLint num_spot = -1;
        GLint spot_pos = -1;
        GLint spot_color = -1;
        GLint spot_dir = -1;
        GLint spot_range = -1;
        GLint spot_inner = -1;
        GLint spot_outer = -1;
    };

    FastUniforms& fast_uniforms(ShaderProgram& program) {
        static std::unordered_map<GLuint, FastUniforms> cache;
        auto [it, inserted] = cache.try_emplace(program.handle);
        if (!inserted) return it->second;

        FastUniforms& u = it->second;
        auto loc = [&program](const char* n) { return shader_uniform(program, n); };
        u.model = loc("u_model");
        u.normal_mat = loc("u_normal_mat");
        u.view = loc("u_view");
        u.proj = loc("u_proj");
        u.view_pos = loc("u_view_pos");
        u.albedo = loc("u_albedo");
        u.emissive = loc("u_emissive");
        u.shininess = loc("u_shininess");
        u.specular = loc("u_specular");
        u.opacity = loc("u_opacity");
        u.unlit = loc("u_unlit");
        u.fog_enabled = loc("u_fog_enabled");
        u.fog_color = loc("u_fog_color");
        u.fog_start = loc("u_fog_start");
        u.fog_end = loc("u_fog_end");
        u.texture = loc("u_texture");
        u.has_texture = loc("u_has_texture");
        u.num_dir = loc("u_num_dir");
        u.dir_dir = loc("u_dir_dir");
        u.dir_color = loc("u_dir_color");
        u.num_point = loc("u_num_point");
        u.point_pos = loc("u_point_pos");
        u.point_color = loc("u_point_color");
        u.point_range = loc("u_point_range");
        u.num_spot = loc("u_num_spot");
        u.spot_pos = loc("u_spot_pos");
        u.spot_color = loc("u_spot_color");
        u.spot_dir = loc("u_spot_dir");
        u.spot_range = loc("u_spot_range");
        u.spot_inner = loc("u_spot_inner");
        u.spot_outer = loc("u_spot_outer");
        return u;
    }

    // Raw uniform upload fast paths (skip name lookup entirely).
    inline void u_mat4(GLint loc, const glm::mat4& m) {
        if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
    }
    inline void u_mat3(GLint loc, const glm::mat3& m) {
        if (loc >= 0) glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
    }
    inline void u_vec3(GLint loc, const glm::vec3& v) {
        if (loc >= 0) glUniform3fv(loc, 1, glm::value_ptr(v));
    }
    inline void u_float(GLint loc, float v) {
        if (loc >= 0) glUniform1f(loc, v);
    }
    inline void u_int(GLint loc, int v) {
        if (loc >= 0) glUniform1i(loc, v);
    }

    GLenum to_gl_mode(cmp::DrawMode mode) {
        switch (mode) {
            case cmp::DrawMode::Lines: return GL_LINES;
            case cmp::DrawMode::LineStrip: return GL_LINE_STRIP;
            case cmp::DrawMode::Points: return GL_POINTS;
            case cmp::DrawMode::TriangleStrip: return GL_TRIANGLE_STRIP;
            default: return GL_TRIANGLES;
        }
    }

    void collect_lights(entt::registry& r, LightFrame& out) {
        for (auto [e, light] : r.view<cmp::DirectionalLight>().each()) {
            if (out.num_dir >= MAX_DIR_LIGHTS) break;
            int i = out.num_dir++;

            glm::vec3 dir = light.direction;
            if (auto* g = r.try_get<cmp::GlobalTransform3D>(e))
                dir = glm::mat3(g->matrix) * dir;
            dir = glm::normalize(dir + glm::vec3(1e-9f));

            out.dir_dir[i] = dir;
            out.dir_color[i] = light.color * light.intensity;
        }

        for (auto [e, light] : r.view<cmp::PointLight>().each()) {
            if (out.num_point >= MAX_POINT_LIGHTS) break;
            if (light.range <= 0.f) continue;
            int i = out.num_point++;

            glm::vec3 pos(0.f);
            if (auto* g = r.try_get<cmp::GlobalTransform3D>(e))
                pos = position_of(g->matrix);
            else if (auto* t = r.try_get<cmp::Transform3D>(e))
                pos = t->position;

            out.point_pos[i] = pos;
            out.point_color[i] = light.color * light.intensity;
            out.point_range[i] = light.range;
        }

        for (auto [e, light] : r.view<cmp::SpotLight>().each()) {
            if (out.num_spot >= MAX_SPOT_LIGHTS) break;
            if (light.range <= 0.f) continue;
            int i = out.num_spot++;

            glm::vec3 pos(0.f);
            glm::vec3 dir = light.direction;
            if (auto* g = r.try_get<cmp::GlobalTransform3D>(e)) {
                pos = position_of(g->matrix);
                dir = glm::mat3(g->matrix) * dir;
            } else if (auto* t = r.try_get<cmp::Transform3D>(e)) {
                pos = t->position;
            }
            dir = glm::normalize(dir + glm::vec3(1e-9f));

            out.spot_pos[i] = pos;
            out.spot_color[i] = light.color * light.intensity;
            out.spot_dir[i] = dir;
            out.spot_range[i] = light.range;
            out.spot_inner[i] = light.inner_angle_deg;
            out.spot_outer[i] = std::max(light.outer_angle_deg, light.inner_angle_deg);
        }
    }

    void upload_lights(ShaderProgram& program, FastUniforms& u, const LightFrame& lights) {
        u_int(u.num_dir, lights.num_dir);
        u_int(u.num_point, lights.num_point);
        u_int(u.num_spot, lights.num_spot);

        if (lights.num_dir > 0) {
            glUniform3fv(u.dir_dir, lights.num_dir, glm::value_ptr(lights.dir_dir[0]));
            glUniform3fv(u.dir_color, lights.num_dir, glm::value_ptr(lights.dir_color[0]));
        }
        if (lights.num_point > 0) {
            glUniform3fv(u.point_pos, lights.num_point, glm::value_ptr(lights.point_pos[0]));
            glUniform3fv(u.point_color, lights.num_point, glm::value_ptr(lights.point_color[0]));
            glUniform1fv(u.point_range, lights.num_point, lights.point_range);
        }
        if (lights.num_spot > 0) {
            glUniform3fv(u.spot_pos, lights.num_spot, glm::value_ptr(lights.spot_pos[0]));
            glUniform3fv(u.spot_color, lights.num_spot, glm::value_ptr(lights.spot_color[0]));
            glUniform3fv(u.spot_dir, lights.num_spot, glm::value_ptr(lights.spot_dir[0]));
            glUniform1fv(u.spot_range, lights.num_spot, lights.spot_range);
            glUniform1fv(u.spot_inner, lights.num_spot, lights.spot_inner);
            glUniform1fv(u.spot_outer, lights.num_spot, lights.spot_outer);
        }
    }

    // World transform of an entity: GlobalTransform3D when present
    // (hierarchy), otherwise composed straight from Transform3D.
    glm::mat4 world_of(entt::registry& r, entt::entity e) {
        if (auto* g = r.try_get<cmp::GlobalTransform3D>(e)) return g->matrix;
        if (auto* t = r.try_get<cmp::Transform3D>(e))
            return compose_transform(t->position, t->rotation, t->scale);
        return glm::mat4(1.f);
    }

    struct RenderEntry {
        entt::entity entity{};
        cmp::MeshRenderer* renderer = nullptr;
        cmp::Material3D* material = nullptr; // may be nullptr -> default material
        glm::mat4 world{1.f};
        float dist = 0.f; // camera-space distance (transparent sorting)
    };

    // Reusable per-frame storage (kept at capacity between frames).
    std::vector<RenderEntry>& opaque_queue() {
        static std::vector<RenderEntry> q;
        return q;
    }
    std::vector<RenderEntry>& transparent_queue() {
        static std::vector<RenderEntry> q;
        return q;
    }

    // Tracks GL state between draws to skip redundant calls.
    struct GlState {
        GLuint bound_texture = 0xFFFFFFFF;
        bool cull_enabled = true;
        ShaderProgram* program = nullptr;

        void bind_texture(GLuint handle) {
            if (handle == bound_texture) return;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, handle);
            bound_texture = handle;
        }
        void set_cull(bool enabled) {
            if (enabled == cull_enabled) return;
            if (enabled) glEnable(GL_CULL_FACE);
            else glDisable(GL_CULL_FACE);
            cull_enabled = enabled;
        }
    };

    void draw_entry(FastUniforms& u, GlState& gl, const RenderEntry& entry) {
        const cmp::Material3D& mat = entry.material ? *entry.material : cmp::Material3D{};

        // Per-object state.
        u_mat4(u.model, entry.world);
        u_mat3(u.normal_mat, glm::mat3(glm::transpose(glm::inverse(entry.world))));

        u_vec3(u.albedo, mat.albedo);
        u_float(u.emissive, mat.emissive);
        u_float(u.shininess, mat.shininess);
        u_float(u.specular, mat.specular);
        u_float(u.opacity, mat.opacity);
        u_int(u.unlit, mat.unlit ? 1 : 0);

        GLuint texture_handle = mat.texture ? mat.texture->res.getNativeHandle() : 0;
        gl.bind_texture(texture_handle);
        u_int(u.has_texture, texture_handle ? 1 : 0);

        gl.set_cull(!mat.two_sided);

        mesh_draw(*entry.renderer->mesh, to_gl_mode(entry.renderer->draw_mode));
    }

    void run() {
        auto& reg = MainLoop::get_registry();
        auto& win = MainLoop::get_window();

        // Zero cost for 2D-only projects: nothing renders without meshes and
        // a current camera.
        if (reg.view<cmp::MeshRenderer>().empty()) return;
        cmp::Camera3D* camera = camera3d_get_current(reg);
        if (!camera) return;

        auto& scene = ensure_scene3d(reg);

        // Camera matrices (entity is authoritative; the transform system has
        // already computed GlobalTransform3D for this frame).
        entt::entity camera_entity = entt::null;
        for (auto [e, c] : reg.view<cmp::Camera3D>().each()) {
            if (&c == camera) camera_entity = e;
        }
        glm::mat4 camera_world = camera_entity != entt::null
            ? world_of(reg, camera_entity)
            : glm::mat4(1.f);

        auto size = win.getSize();
        float aspect = size.y > 0 ? static_cast<float>(size.x) / static_cast<float>(size.y) : 1.f;

        if (camera->perspective) {
            camera->projection = glm::perspective(
                glm::radians(camera->fov_deg), aspect,
                camera->near_z, camera->far_z);
        } else {
            float s = camera->ortho_size;
            camera->projection = glm::ortho(
                -s * aspect, s * aspect, -s, s,
                camera->near_z, camera->far_z);
        }
        camera->view = glm::inverse(camera_world);

        scene.view = camera->view;
        scene.projection = camera->projection;
        scene.view_pos = position_of(camera_world);
        scene.has_camera = true;

        ShaderProgramRef default_prog = ensure_default_program();
        if (!default_prog) return; // shader failed to compile

        // Collect lights (reused storage, no allocations).
        static LightFrame lights;
        lights.clear();
        collect_lights(reg, lights);

        // Collect renderables (reused storage, no allocations).
        auto& opaque = opaque_queue();
        auto& transparent = transparent_queue();
        opaque.clear();
        transparent.clear();

        for (auto [e, renderer] : reg.view<cmp::MeshRenderer>().each()) {
            if (!renderer.visible || !renderer.mesh) continue;

            glm::mat4 world = world_of(reg, e);
            cmp::Material3D* material = reg.try_get<cmp::Material3D>(e);
            float dist = glm::length(position_of(world) - scene.view_pos);

            if (material && material->opacity < 1.f)
                transparent.push_back({e, &renderer, material, world, dist});
            else
                opaque.push_back({e, &renderer, material, world, dist});
        }

        if (opaque.empty() && transparent.empty()) return;

        // Opaque: state-sorted (program, texture, mesh) to minimize switches.
        // Transparent: back-to-front.
        if (opaque.size() > 1) {
            std::sort(opaque.begin(), opaque.end(), [](const RenderEntry& a, const RenderEntry& b) {
                GLuint pa = a.renderer->shader ? a.renderer->shader->program->handle : 0;
                GLuint pb = b.renderer->shader ? b.renderer->shader->program->handle : 0;
                if (pa != pb) return pa < pb;

                GLuint ta = (a.material && a.material->texture) ? a.material->texture->res.getNativeHandle() : 0;
                GLuint tb = (b.material && b.material->texture) ? b.material->texture->res.getNativeHandle() : 0;
                if (ta != tb) return ta < tb;

                return a.renderer->mesh.operator->() < b.renderer->mesh.operator->();
            });
        }
        if (transparent.size() > 1) {
            std::sort(transparent.begin(), transparent.end(),
                      [](const RenderEntry& a, const RenderEntry& b) {
                          return a.dist > b.dist;
                      });
        }

        // 3D pass setup.
        glViewport(0, 0, static_cast<GLint>(size.x), static_cast<GLint>(size.y));

        GLbitfield clear_mask = 0;
        if (scene.clear_color) {
            glClearColor(scene.clear_color_value.r, scene.clear_color_value.g,
                         scene.clear_color_value.b, 1.f);
            clear_mask |= GL_COLOR_BUFFER_BIT;
        }
        if (scene.clear_depth) {
            glClearDepth(1.f);
            clear_mask |= GL_DEPTH_BUFFER_BIT;
        }
        if (clear_mask) glClear(clear_mask);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glFrontFace(GL_CCW);
        glPolygonMode(GL_FRONT_AND_BACK, scene.wireframe ? GL_LINE : GL_FILL);

        GlState gl;

        // Per-frame state is uploaded once per program switch.
        auto prepare_program = [&](ShaderProgram& program) {
            if (gl.program == &program) return;
            gl.program = &program;
            shader_use(program);

            FastUniforms& u = fast_uniforms(program);
            u_mat4(u.view, scene.view);
            u_mat4(u.proj, scene.projection);
            u_vec3(u.view_pos, scene.view_pos);

            u_int(u.fog_enabled, scene.fog_enabled ? 1 : 0);
            u_vec3(u.fog_color, scene.fog_color);
            u_float(u.fog_start, scene.fog_start);
            u_float(u.fog_end, scene.fog_end);

            u_int(u.texture, 0);
            upload_lights(program, u, lights);
        };

        // Opaque queue.
        glDisable(GL_BLEND);
        for (const auto& entry : opaque) {
            ShaderProgram& program = entry.renderer->shader
                ? *entry.renderer->shader->program
                : *default_prog;
            prepare_program(program);
            draw_entry(fast_uniforms(program), gl, entry);
        }

        // Transparent queue.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for (const auto& entry : transparent) {
            ShaderProgram& program = entry.renderer->shader
                ? *entry.renderer->shader->program
                : *default_prog;
            prepare_program(program);
            draw_entry(fast_uniforms(program), gl, entry);
        }

        // Restore state so SFML's 2D renderer keeps working after this pass.
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        win.resetGLStates();
    }
}

ME_REGISTER_SYSTEM(
    "engine:3d:render",
    me::SystemPhase::Render,
    std::numeric_limits<int>::min() + 2, // before tilemap (min+3) and sprites (min+6)
    run
)
