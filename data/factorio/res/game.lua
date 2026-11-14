F = {}

F.active = false
F.role = "single"
F.my_peer = 0
F.selected = nil
F.dir = 2
F.hover_cell = nil
F.inspect_cell = nil
F.world_ready = false
F.seed = 0
F.entities = {}
F.protos = {}
F.items = {}
F.recipes = {}
F.technologies = {}
F.tiles = {}
F.tile_index = {}
F.visual = {}
F.world = { tilemap = nil }
F.inventory = {}
F.research = { level = 0, units = 0, active = "", researched = {} }
F.production = { ore = 0, plates = 0, science = 0, crafted = 0 }
F.stats = { buildings = 0 }
F.zoom = 2.0
F.victory_shown = false

local CELL = FConfig.cell

function F.key(x, y) return y * 1024 + x end
function F.key_x(k) return k % 1024 end
function F.key_y(k) return math.floor(k / 1024) end

function F.cell_center(x, y)
    return x * CELL + CELL * 0.5, y * CELL + CELL * 0.5
end

function F.world_to_cell(wx, wy)
    return math.floor(wx / CELL), math.floor(wy / CELL)
end

function F.registry() return MainLoop.get_registry() end

function F.notify(text)
    if FUI and FUI.toast then FUI.toast(text) end
    print("[Factorio] " .. text)
end

function F.play_sound(name)
    local sounds = {
        place = "data/factorio/res/sounds/place.wav",
        craft = "data/factorio/res/sounds/craft.wav",
        research = "data/factorio/res/sounds/research.wav",
        victory = "data/factorio/res/sounds/victory.wav",
    }
    local path = sounds[name]
    if not path then return end
    if not SoundPlayer or not ResourceLoader then return end
    pcall(function()
        SoundPlayer.play(ResourceLoader.load_SoundBuffer(path))
    end)
end

function F.spawn_entity(id, wx, wy)
    local r = F.registry()
    local e = PackedEntity.spawn(id, r)
    if e == nil then return nil end
    if not r:valid(e) then return nil end
    local t = r:get_Transform(e)
    if t then t.position = Vector2f.new(wx, wy) end
    return e
end

function F.destroy_entity(e)
    if e == nil then return end
    local r = F.registry()
    if r:valid(e) then r:destroy(e) end
end

function F.refresh_catalog()
    F.protos = {}
    for _, proto in ipairs(Factory.control.entity_prototypes()) do
        F.protos[proto.name] = proto
    end

    F.items = {}
    for _, item in ipairs(Factory.control.items()) do
        F.items[item.name] = item
    end

    F.recipes = {}
    for _, recipe in ipairs(Factory.control.recipes()) do
        F.recipes[recipe.name] = recipe
    end

    F.technologies = {}
    for _, tech in ipairs(Factory.control.technologies()) do
        F.technologies[tech.name] = tech
    end

    F.tiles = {}
    F.tile_index = {}
    local index = 0
    for _, tile in ipairs(Factory.control.tiles()) do
        F.tiles[tile.name] = tile
        F.tile_index[tile.name] = index
        index = index + 1
    end
end

function F.placeable_entities()
    local list = {}
    for _, proto in ipairs(Factory.control.entity_prototypes()) do
        if proto.placeable ~= false then
            list[#list + 1] = proto
        end
    end
    table.sort(list, function(a, b) return a.name < b.name end)
    return list
end

function F.build_order()
    local order = {
        "factorio:drill",
        "factorio:furnace",
        "factorio:belt",
        "factorio:inserter",
        "factorio:chest",
        "factorio:assembler",
        "factorio:lab",
    }
    local list = {}
    for _, name in ipairs(order) do
        local proto = F.protos[name]
        if proto then list[#list + 1] = proto end
    end
    return list
end

function F.craft_order()
    local list = {}
    for _, recipe in ipairs(Factory.control.recipes()) do
        list[#list + 1] = recipe
    end
    return list
end

function F.sync()
    F.entities = {}
    F.stats.buildings = 0
    for _, entity in ipairs(Factory.control.entities()) do
        local cell = F.key(entity.x, entity.y)
        F.entities[cell] = entity
        F.stats.buildings = F.stats.buildings + 1
    end

    F.inventory = {}
    for _, entry in ipairs(Factory.control.inventory()) do
        F.inventory[entry.item] = entry.count
    end

    local research = Factory.control.research()
    F.research.level = research.level
    F.research.units = research.units
    F.research.active = research.active
    F.research.researched = research.researched
end

function F.inv_count(item) return F.inventory[item] or 0 end

function F.entity_at(x, y) return F.entities[F.key(x, y)] end

function F.inspect_info()
    if not F.inspect_cell then return nil end
    local entity = F.entities[F.inspect_cell]
    if entity == nil then
        F.inspect_cell = nil
        return nil
    end
    local proto = F.protos[entity.name] or {}
    return {
        cell = F.inspect_cell,
        kind = entity.name,
        label = proto.label or entity.name,
        dir = entity.dir,
        recipe = entity.recipe,
        progress = entity.progress,
        inv = entity.input,
        out_item = entity.output,
        out_count = entity.held and entity.held.item or nil,
        held = entity.held,
    }
end

function F.can_place(x, y)
    if F.entity_at(x, y) then return false end
    return Factory.control.buildable(x, y)
end

function F.in_reach(x, y)
    local cell = F.my_cell()
    if not cell then return true end
    local dx = x - cell.x
    local dy = y - cell.y
    return dx * dx + dy * dy <= 12 * 12
end

function F.my_cell()
    local r = F.registry()
    local found = nil
    r:foreach_Transform(function(e)
        if found == nil and r:has_PlayerInput(e) and Network.is_authority(e) then
            found = e
        end
    end)
    if found == nil then return nil end
    if not r:valid(found) then return nil end
    local t = r:get_Transform(found)
    local x, y = F.world_to_cell(t.position.x, t.position.y)
    return { x = x, y = y }
end

function F.spawn_position(pid)
    local width = Factory.control.width()
    local height = Factory.control.height()
    local cx = math.floor(width * 0.5)
    local cy = math.floor(height * 0.5)
    local slot = (pid or 0) % 8
    cx = cx + (slot % 4) * 2
    cy = cy + math.floor(slot / 4) * 2
    return F.cell_center(cx, cy)
end

function F.set_player_spawn(e, pid)
    if e == nil then return end
    local r = F.registry()
    if not r:valid(e) then return end
    local t = r:get_Transform(e)
    if not t then return end
    local wx, wy = F.spawn_position(pid)
    t.position = Vector2f.new(wx, wy)
end
