#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/Resources.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::TileSet>("TileSet",
        sol::no_constructor,
        "tile_size", &me::TileSet::tile_size,
        "y_sort_origin", &me::TileSet::y_sort_origin,
        "size", sol::property([](me::TileSet& ts) { return ts.size(); })
    );
   
    sol::table rl = lua["ResourceLoader"];
    me::lua::register_resource<me::TileSet>(rl, "TileSet");
}

ME_LUA_BINDING(
    "engine:TileSet",
    static_cast<unsigned>(me::lua::BindPriority::Resource),
    &bind
)