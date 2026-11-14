FWorldgen = {}

local function tile_name_at(x, y)
    return Factory.control.tile_at(x, y)
end

local function is_ore(name)
    local tile = F.tiles[name]
    return tile ~= nil and tile.ore_item ~= nil and tile.ore_item ~= ""
end

local function is_water(name)
    local tile = F.tiles[name]
    return tile ~= nil and tile.buildable == false
end

function FWorldgen.build()
    local r = F.registry()

    local e = r:create()
    r:add_Transform(e)
    local tm = r:add_TileMap(e)
    tm.tileset = ResourceLoader.load_TileSet("data/factorio/res/tilesets/terrain.json")

    local width = Factory.control.width()
    local height = Factory.control.height()
    local map = Factory.control.map()

    local tiles = {}
    for i = 1, #map do
        local index = F.tile_index[map[i]]
        tiles[i] = index or 0
    end

    tm:set_tiles(width, height, tiles)
    F.world.tilemap = e
    F.world_ready = true
    return e
end

function FWorldgen.despawn()
    if F.world.tilemap ~= nil then
        F.destroy_entity(F.world.tilemap)
        F.world.tilemap = nil
    end
    F.world_ready = false
end

function FWorldgen.decor()
    local width = Factory.control.width()
    local height = Factory.control.height()
    local cx = math.floor(width * 0.5)
    local cy = math.floor(height * 0.5)
    local min_distance = FConfig.decor.min_distance_from_spawn

    math.randomseed(F.seed * 7919 + 17)

    local function place(count, texture, scale_range)
        local placed = 0
        local attempts = 0
        while placed < count and attempts < count * 40 do
            attempts = attempts + 1
            local x = math.random(1, width - 2)
            local y = math.random(1, height - 2)
            local dx = x - cx
            local dy = y - cy
            local too_close = dx * dx + dy * dy < min_distance * min_distance
            local name = tile_name_at(x, y)
            local blocked = name == nil or name == "" or is_ore(name) or is_water(name)
                or F.entity_at(x, y) ~= nil

            if not too_close and not blocked then
                local wx, wy = F.cell_center(x, y)
                local e = F.spawn_entity("factory:decor", wx + math.random() * 12 - 6, wy + math.random() * 12 - 6)
                if e ~= nil then
                    local r = F.registry()
                    local sp = r:get_Sprite(e)
                    if sp ~= nil then sp.texture = ResourceLoader.load_Texture(texture) end
                    local t = r:get_Transform(e)
                    if t ~= nil then
                        local s = scale_range[1] + math.random() * (scale_range[2] - scale_range[1])
                        t.scale = Vector2f.new(s, s)
                    end
                    FVisuals.decor[#FVisuals.decor + 1] = e
                    placed = placed + 1
                end
            end
        end
    end

    place(FConfig.decor.trees, "data/factorio/res/textures/decor_tree.png", { 0.85, 1.15 })
    place(FConfig.decor.bushes, "data/factorio/res/textures/decor_bush.png", { 0.8, 1.1 })
end
