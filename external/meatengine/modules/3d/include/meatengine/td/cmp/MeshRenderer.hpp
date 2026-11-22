#pragma once

#include <entt/resource/resource.hpp>

#include <meatengine/td/Mesh.hpp>
#include <meatengine/td/Shader3D.hpp>

namespace me::td::cmp {

    // Primitive topology used to draw the mesh.
    enum class DrawMode : uint32_t {
        Triangles = 0,
        Lines,
        LineStrip,
        Points,
        TriangleStrip
    };

    // Makes an entity render a mesh in the 3D pass. The world transform is
    // taken from GlobalTransform3D (computed automatically from Transform3D).
    struct MeshRenderer {
        // Empty mesh -> entity is not drawn.
        entt::resource<Mesh> mesh{};

        // Optional per-renderer shader override; otherwise the built-in
        // Blinn-Phong forward shader is used.
        entt::resource<Shader3D> shader{};

        DrawMode draw_mode = DrawMode::Triangles;
        bool visible = true;
    };
}
