-- tilemap.tileset = me::ResourceLoader::load<me::TileSet>("data/core/res/tilesets/tileset.json");
-- tilemap.load_tiles("data/core/res/tilemaps/tilemap.json");

ScriptedStateRegistry.register("core:world", {
    on_enter = function(w, r) 
        local player = PackedEntity.spawn("core:player", MainLoop.get_registry())

        local e = r:create()
        r:add_Transform(e)
        local tilemap = r:add_TileMap(e)
        tilemap.tileset = ResourceLoader.load_TileSet("data/core/res/tilesets/tileset.json")
        tilemap:load_tiles("data/core/res/tilemaps/tilemap.json")
        
    end,

    --update = function(w, r, dt)

    --end,
})
