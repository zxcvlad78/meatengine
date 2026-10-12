#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/TileMap.hpp>

static void bind(sol::state& lua) {
    auto tilemap_ut = lua.new_usertype<me::cmp::TileMap>("TileMap",
        sol::constructors<me::cmp::TileMap()>(),
        "origin_x", &me::cmp::TileMap::origin_x,
        "origin_y", &me::cmp::TileMap::origin_y,
        "width", &me::cmp::TileMap::width,
        "height", &me::cmp::TileMap::height,
        "dirty", &me::cmp::TileMap::dirty,
        "tiles", &me::cmp::TileMap::tiles,
        "tileset", sol::property(
            [](me::cmp::TileMap& tm) -> me::TileSet* {
                auto h = tm.tileset.handle();
                return h ? h.get() : nullptr;
            },
            [](me::cmp::TileMap& tm, me::TileSet& ts) {
                auto sp = me::ResourceLoader::find_handle(&ts);
                if (!sp) {
                    throw std::runtime_error(
                        "TileMap.tileset: TileSet not from ResourceLoader "
                        "(load it via ResourceLoader.load_tileset first)");
                }
                tm.tileset = entt::resource<me::TileSet>{sp};
                tm.dirty = true;
            }
        )
    );

    tilemap_ut.set_function("load_tiles", &me::cmp::TileMap::load_tiles);
    tilemap_ut.set_function("set_tile", &me::cmp::TileMap::set_tile);
    tilemap_ut.set_function("get_tile", &me::cmp::TileMap::get_tile);
    
    me::lua::register_component<me::cmp::TileMap>("TileMap", lua);
}

ME_LUA_BINDING(
    "engine:cmp:TileMap",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)