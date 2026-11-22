#include <sol/sol.hpp>

#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/td/Mesh.hpp>

// Lua handle for meshes produced by the td API (td.cube, td.load_mesh, ...).

static void bind(sol::state& lua) {
    lua.new_usertype<me::td::Mesh>("MeshRef",
        sol::no_constructor,
        "vertex_count", sol::readonly(&me::td::Mesh::vertices),
        "index_count", sol::property(
            [](const me::td::Mesh& m) { return m.indices.size(); })
    );
}

ME_LUA_BINDING(
    "engine:3d:resource:MeshRef",
    static_cast<unsigned>(me::lua::BindPriority::Resource),
    &bind
)
