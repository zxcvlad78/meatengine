#include <meatengine/td/DefaultShader.hpp>

#include <meatengine/td/ShaderOps.hpp>

namespace me::td {

    namespace {
        // Limits must match the render pass (see systems/render3d.cpp).
        constexpr int MAX_DIR_LIGHTS = 4;
        constexpr int MAX_POINT_LIGHTS = 16;
        constexpr int MAX_SPOT_LIGHTS = 8;

        const char* DEFAULT_VERTEX_SRC = R"(#version 330 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform mat3 u_normal_mat;

out vec3 v_world_pos;
out vec3 v_normal;
out vec2 v_uv;

void main() {
    vec4 world = u_model * vec4(a_position, 1.0);
    v_world_pos = world.xyz;
    v_normal = u_normal_mat * a_normal;
    v_uv = a_uv;
    gl_Position = u_proj * u_view * world;
}
)";

        const char* DEFAULT_FRAGMENT_SRC = R"(#version 330 core
in vec3 v_world_pos;
in vec3 v_normal;
in vec2 v_uv;
out vec4 frag_color;

uniform vec3 u_view_pos;
uniform vec3 u_albedo;
uniform bool u_has_texture;
uniform sampler2D u_texture;
uniform float u_emissive;
uniform float u_shininess;
uniform float u_specular;
uniform float u_opacity;
uniform bool u_unlit;

uniform vec3 u_fog_color;
uniform bool u_fog_enabled;
uniform float u_fog_start;
uniform float u_fog_end;

uniform int u_num_dir;
uniform vec3 u_dir_dir[4];
uniform vec3 u_dir_color[4];

uniform int u_num_point;
uniform vec3 u_point_pos[16];
uniform vec3 u_point_color[16];
uniform float u_point_range[16];

uniform int u_num_spot;
uniform vec3 u_spot_pos[8];
uniform vec3 u_spot_color[8];
uniform vec3 u_spot_dir[8];
uniform float u_spot_range[8];
uniform float u_spot_inner[8];
uniform float u_spot_outer[8];

vec3 blinn_phong(vec3 n, vec3 v, vec3 diffuse, vec3 light_color, vec3 l) {
    float nd = max(dot(n, l), 0.0);
    vec3 h = normalize(v + l);
    float sp = pow(max(dot(n, h), 0.0), u_shininess) * u_specular;
    return light_color * (diffuse * nd + diffuse * sp);
}

void main() {
    vec4 texel = u_has_texture ? texture(u_texture, v_uv) : vec4(1.0);
    vec3 diffuse = u_albedo * texel.rgb;
    vec3 n = normalize(v_normal);
    vec3 v = normalize(u_view_pos - v_world_pos);

    vec3 color;
    if (u_unlit) {
        color = diffuse;
    } else {
        // Ambient term.
        color = diffuse * 0.15;

        for (int i = 0; i < u_num_dir; ++i) {
            color += blinn_phong(n, v, diffuse, u_dir_color[i], normalize(-u_dir_dir[i]));
        }

        for (int i = 0; i < u_num_point; ++i) {
            vec3 d = u_point_pos[i] - v_world_pos;
            float dist = length(d);
            vec3 l = d / max(dist, 0.0001);
            float att = clamp(1.0 - dist / u_point_range[i], 0.0, 1.0);
            att *= att;
            color += blinn_phong(n, v, diffuse, u_point_color[i] * att, l);
        }

        for (int i = 0; i < u_num_spot; ++i) {
            vec3 d = u_spot_pos[i] - v_world_pos;
            float dist = length(d);
            vec3 l = d / max(dist, 0.0001);
            float cutoff = dot(-l, normalize(u_spot_dir[i]));
            float inner = cos(radians(u_spot_inner[i]));
            float outer = cos(radians(u_spot_outer[i]));
            if (cutoff <= outer) continue;
            float spot = clamp((cutoff - outer) / (inner - outer + 0.00001), 0.0, 1.0);
            float att = clamp(1.0 - dist / u_spot_range[i], 0.0, 1.0);
            att *= att;
            color += blinn_phong(n, v, diffuse, u_spot_color[i] * att * spot, l);
        }
    }

    color += diffuse * u_emissive;

    if (u_fog_enabled) {
        float dist = length(u_view_pos - v_world_pos);
        float f = clamp((dist - u_fog_start) / (u_fog_end - u_fog_start), 0.0, 1.0);
        color = mix(color, u_fog_color, f);
    }

    frag_color = vec4(color, u_opacity * texel.a);
}
)";
    }

    ShaderProgramRef& default_program() {
        static ShaderProgramRef program;
        return program;
    }

    // Compiled lazily on first call; the GL context must be current.
    ShaderProgramRef& ensure_default_program() {
        ShaderProgramRef& program = default_program();
        if (!program) {
            program = compile_program(DEFAULT_VERTEX_SRC, DEFAULT_FRAGMENT_SRC);
        }
        return program;
    }
}
