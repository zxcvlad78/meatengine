#include <sol/sol.hpp>

#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/td/RendererOps.hpp>
#include <meatengine/td/cmp/MeshRenderer.hpp>

static void bind(sol::state& lua) {
    // DrawMode constants: DrawMode.TRIANGLES / .LINES / .LINE_STRIP / .POINTS / .TRIANGLE_STRIP
    lua["DrawMode"] = lua.create_table_with(
        "TRIANGLES", static_cast<int>(me::td::cmp::DrawMode::Triangles),
        "LINES", static_cast<int>(me::td::cmp::DrawMode::Lines),
        "LINE_STRIP", static_cast<int>(me::td::cmp::DrawMode::LineStrip),
        "POINTS", static_cast<int>(me::td::cmp::DrawMode::Points),
        "TRIANGLE_STRIP", static_cast<int>(me::td::cmp::DrawMode::TriangleStrip)
    );

    lua.new_usertype<me::td::cmp::MeshRenderer>("MeshRenderer",
        sol::constructors<me::td::cmp::MeshRenderer()>(),
        "visible", &me::td::cmp::MeshRenderer::visible,
        "draw_mode", sol::property(
            [](const me::td::cmp::MeshRenderer& r) { return static_cast<int>(r.draw_mode); },
            [](me::td::cmp::MeshRenderer& r, int mode) {
                r.draw_mode = static_cast<me::td::cmp::DrawMode>(mode);
            }
        ),
        "mesh", sol::property(
            [](me::td::cmp::MeshRenderer& r) -> me::td::MeshRef {
                return r.mesh.handle();
            },
            [](me::td::cmp::MeshRenderer& r, me::td::MeshRef mesh) {
                me::td::renderer_set_mesh(r, std::move(mesh));
            }
        )
    );

    me::lua::register_component<me::td::cmp::MeshRenderer>("MeshRenderer", lua);
}

ME_LUA_BINDING(
    "engine:3d:cmp:MeshRenderer",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)
