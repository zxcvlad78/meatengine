#include <string>
#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/assets/ResourceLoader.hpp>

#include <meatengine/graphics/cmp/TileMap.hpp>

static void bind(sol::state& lua) {
    auto tilemap_ut = lua.new_usertype<me::graphics::cmp::TileMap>("TileMap",
        sol::constructors<me::graphics::cmp::TileMap()>(),
        "origin_x", &me::graphics::cmp::TileMap::origin_x,
        "origin_y", &me::graphics::cmp::TileMap::origin_y,
        "width", &me::graphics::cmp::TileMap::width,
        "height", &me::graphics::cmp::TileMap::height,
        "dirty", &me::graphics::cmp::TileMap::dirty,
        "tiles", &me::graphics::cmp::TileMap::tiles,
        "tileset", sol::property(
            [](me::graphics::cmp::TileMap& tm) -> me::TileSet* {
                auto h = tm.tileset.handle();
                return h ? h.get() : nullptr;
            },
            [](me::graphics::cmp::TileMap& tm, me::TileSet& ts) {
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

    tilemap_ut.set_function("load_tiles", &me::graphics::cmp::TileMap::load_tiles);
    tilemap_ut.set_function("set_tile", &me::graphics::cmp::TileMap::set_tile);
    tilemap_ut.set_function("get_tile", &me::graphics::cmp::TileMap::get_tile);

    tilemap_ut.set_function("set_tiles",
        [](me::graphics::cmp::TileMap& tm, unsigned int width, unsigned int height, sol::table tiles) {
            tm.origin_x = 0;
            tm.origin_y = 0;
            tm.width = width;
            tm.height = height;
            tm.tiles.assign(static_cast<std::size_t>(width) * height, -1);

            for (std::size_t i = 0; i < tm.tiles.size(); ++i) {
                sol::object v = tiles.get<sol::object>(static_cast<int>(i + 1));
                if (v.is<int>()) tm.tiles[i] = v.as<int>();
            }

            tm.dirty = true;
        });
    
    me::lua::register_component<me::graphics::cmp::TileMap>("TileMap", lua);
}

ME_LUA_BINDING(
    "engine:cmp:TileMap",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)