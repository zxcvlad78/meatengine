FVisuals = {}

FVisuals.overlays = {
    ["factorio:overlay_gear"] = "data/factorio/res/textures/overlay_gear.png",
    ["factorio:overlay_gear_gold"] = "data/factorio/res/textures/overlay_gear_gold.png",
    ["factorio:overlay_flame"] = "data/factorio/res/textures/overlay_flame.png",
    ["factorio:furnace_glow"] = "data/factorio/res/textures/furnace_glow.png",
    ["factorio:particle_glow_blue"] = "data/factorio/res/textures/particle_glow_blue.png",
}

FVisuals.particles = {
    spark = "data/factorio/res/textures/particle_spark.png",
    smoke = "data/factorio/res/textures/particle_smoke.png",
    glow_blue = "data/factorio/res/textures/particle_glow_blue.png",
    glow_yellow = "data/factorio/res/textures/particle_glow_yellow.png",
}

FVisuals.entities = {}
FVisuals.decor = {}
FVisuals.particles_list = {}
FVisuals.t = 0
FVisuals.ghost = nil
FVisuals.ghost_kind = nil

local MAX_PARTICLES = 140

local function transform_of(e)
    local r = F.registry()
    if not r:valid(e) then return nil end
    return r:get_Transform(e)
end

local function set_texture(e, path)
    local r = F.registry()
    if not r:valid(e) then return end
    local sp = r:get_Sprite(e)
    if sp == nil then return end
    sp.texture = ResourceLoader.load_Texture(path)
end

local function set_color(e, r, g, b, a)
    local reg = F.registry()
    if not reg:valid(e) then return end
    local sp = reg:get_Sprite(e)
    if sp == nil then return end
    local s = sp.sf_sprite
    if s then s:setColor(Color.new(r, g, b, a)) end
end

local function has_capability(proto, name)
    if proto == nil or proto.capabilities == nil then return false end
    for _, capability in ipairs(proto.capabilities) do
        if capability == name then return true end
    end
    return false
end

function FVisuals.reset()
    for cell, visual in pairs(FVisuals.entities) do
        F.destroy_entity(visual.main)
        for _, overlay in ipairs(visual.overlays) do
            F.destroy_entity(overlay)
        end
        F.destroy_entity(visual.item)
    end
    for _, decor in ipairs(FVisuals.decor) do
        F.destroy_entity(decor)
    end
    for _, particle in ipairs(FVisuals.particles_list) do
        F.destroy_entity(particle.e)
    end
    FVisuals.entities = {}
    FVisuals.decor = {}
    FVisuals.particles_list = {}
    FVisuals.ghost = nil
    FVisuals.ghost_kind = nil
    FVisuals.t = 0
end

local function ensure_overlays(cell, visual, proto, wx, wy)
    if proto.overlays == nil then return end
    for _, id in ipairs(proto.overlays) do
        if visual.overlays[id] == nil then
            local overlay = F.spawn_entity("factory:overlay", wx, wy + (proto.overlay_y or 0))
            local path = FVisuals.overlays[id]
            if overlay and path then set_texture(overlay, path) end
            visual.overlays[id] = overlay
        end
    end
end

function FVisuals.sync_entities()
    for cell, entity in pairs(F.entities) do
        local visual = FVisuals.entities[cell]
        local proto = F.protos[entity.name]

        if visual ~= nil and visual.kind ~= entity.name then
            F.destroy_entity(visual.main)
            for _, overlay in pairs(visual.overlays) do F.destroy_entity(overlay) end
            F.destroy_entity(visual.item)
            visual = nil
            FVisuals.entities[cell] = nil
        end

        if visual == nil and proto ~= nil then
            local wx, wy = F.cell_center(F.key_x(cell), F.key_y(cell))
            visual = { kind = entity.name, main = nil, overlays = {}, item = nil, item_name = nil }
            if proto.flat then
                visual.main = F.spawn_entity("factory:entity", wx, wy)
                local t = transform_of(visual.main)
                if t then t.scale = Vector2f.new(1, 1) end
            else
                visual.main = F.spawn_entity("factory:entity", wx, wy)
                local t = transform_of(visual.main)
                if t then
                    t.position = Vector2f.new(wx + (proto.anchor_x or -32), wy + (proto.anchor_y or -64))
                end
            end
            if visual.main and proto.graphics then set_texture(visual.main, proto.graphics) end
            ensure_overlays(cell, visual, proto, wx, wy)
            FVisuals.entities[cell] = visual
        end

        if visual ~= nil then
            if proto and proto.graphics then set_texture(visual.main, proto.graphics) end
            ensure_overlays(cell, visual, proto, F.cell_center(F.key_x(cell), F.key_y(cell)))
        end
    end

    for cell, visual in pairs(FVisuals.entities) do
        if F.entities[cell] == nil then
            F.destroy_entity(visual.main)
            for _, overlay in pairs(visual.overlays) do F.destroy_entity(overlay) end
            F.destroy_entity(visual.item)
            FVisuals.entities[cell] = nil
        end
    end
end

function FVisuals.sync_items()
    for cell, entity in pairs(F.entities) do
        local visual = FVisuals.entities[cell]
        if visual ~= nil then
            local held = entity.held
            local name = (held and held.item ~= "") and held.item or nil
            if name ~= visual.item_name then
                F.destroy_entity(visual.item)
                visual.item = nil
                visual.item_name = name
                if name ~= nil then
                    local item = F.items[name]
                    if item ~= nil and item.icon ~= nil and item.icon ~= "" then
                        local wx, wy = F.cell_center(F.key_x(cell), F.key_y(cell))
                        visual.item = F.spawn_entity("factory:item", wx, wy)
                        set_texture(visual.item, item.icon)
                    end
                end
            end
        end
    end
end

local function add_particle(id, x, y, vx, vy, life, s0, s1, a0, a1, gravity, cr, cg, cb)
    if #FVisuals.particles_list >= MAX_PARTICLES then return end
    local path = FVisuals.particles[id]
    if not path then return end
    local e = F.spawn_entity("factory:particle", x, y)
    if not e then return end
    set_texture(e, path)
    local t = transform_of(e)
    if t then t.scale = Vector2f.new(s0, s0) end
    set_color(e, cr, cg, cb, a0)
    FVisuals.particles_list[#FVisuals.particles_list + 1] = {
        e = e, x = x, y = y, vx = vx, vy = vy,
        life = life, max = life, s0 = s0, s1 = s1,
        a0 = a0, a1 = a1, gravity = gravity, cr = cr, cg = cg, cb = cb,
    }
end

local function update_particles(dt)
    local list = FVisuals.particles_list
    for i = #list, 1, -1 do
        local p = list[i]
        p.life = p.life - dt
        if p.life <= 0 then
            F.destroy_entity(p.e)
            table.remove(list, i)
        else
            p.x = p.x + p.vx * dt
            p.y = p.y + p.vy * dt
            p.vy = p.vy + p.gravity * dt
            local k = 1.0 - p.life / p.max
            local t = transform_of(p.e)
            if t then
                t.position = Vector2f.new(p.x, p.y)
                local s = p.s0 + (p.s1 - p.s0) * k
                t.scale = Vector2f.new(s, s)
            end
            local a = p.a0 + (p.a1 - p.a0) * k
            set_color(p.e, p.cr, p.cg, p.cb, a)
        end
    end
end

local function update_machines(dt)
    for cell, entity in pairs(F.entities) do
        local visual = FVisuals.entities[cell]
        if visual ~= nil then
            local proto = F.protos[entity.name]
            local wx, wy = F.cell_center(F.key_x(cell), F.key_y(cell))
            local busy = (entity.progress or 0) > 0.01

            for id, overlay in pairs(visual.overlays) do
                local t = transform_of(overlay)
                if t ~= nil then
                    if id == "factorio:overlay_gear" then
                        t.rotation_degrees = (t.rotation_degrees + 120 * dt) % 360
                    elseif id == "factorio:overlay_gear_gold" then
                        local swing = entity.swing or 0
                        if has_capability(proto, "inserter") then
                            t.rotation_degrees = swing * 180
                        else
                            t.rotation_degrees = (t.rotation_degrees + 200 * dt) % 360
                        end
                    elseif id == "factorio:furnace_glow" then
                        local pulse = math.sin(FVisuals.t * 3 + (cell % 7))
                        t.scale = Vector2f.new(1 + 0.1 * pulse, 1 + 0.1 * pulse)
                        set_color(overlay, 255, 255, 255, 130 + 40 * pulse)
                    elseif id == "factorio:overlay_flame" then
                        local pulse = 0.9 + 0.18 * math.abs(math.sin(FVisuals.t * 5))
                        t.scale = Vector2f.new(pulse, pulse)
                        set_color(overlay, 255, 255, 255, 150 + 60 * pulse)
                    elseif id == "factorio:particle_glow_blue" then
                        local pulse = math.sin(FVisuals.t * 2 + (cell % 5))
                        t.scale = Vector2f.new(0.9 + 0.1 * pulse, 0.9 + 0.1 * pulse)
                        set_color(overlay, 255, 255, 255, 120 + 40 * pulse)
                    end
                end
            end

            if has_capability(proto, "logistic") and visual.item ~= nil then
                local t = transform_of(visual.item)
                if t ~= nil then
                    local pulse = 0.9 + 0.16 * math.sin(FVisuals.t * 8 + F.key_x(cell))
                    t.scale = Vector2f.new(pulse, pulse)
                    t.rotation_degrees = (t.rotation_degrees + 30 * dt) % 360
                end
            end

            if busy and has_capability(proto, "mining") then
                visual.spark_t = (visual.spark_t or 0) + dt
                if visual.spark_t >= 0.9 then
                    visual.spark_t = 0
                    add_particle("spark", wx + math.random() * 10 - 5, wy - 8,
                        math.random() * 60 - 30, math.random() * 45 - 38,
                        0.35, 1.0, 0.2, 255, 0, 60, 255, 236, 170)
                end
            end

            if busy and has_capability(proto, "crafting") then
                visual.smoke_t = (visual.smoke_t or 0) + dt
                if visual.smoke_t >= 0.55 then
                    visual.smoke_t = 0
                    add_particle("smoke", wx + math.random() * 8 - 4, wy - 24,
                        math.random() * 8 - 4, -(10 + math.random() * 10),
                        1.1 + math.random() * 0.5, 0.5, 1.3, 150, 0, -5,
                        235, 235, 240)
                end
            end
        end
    end
end

local function update_ghost()
    if F.selected == nil then
        if FVisuals.ghost ~= nil then
            F.destroy_entity(FVisuals.ghost)
            FVisuals.ghost = nil
            FVisuals.ghost_kind = nil
        end
        return
    end

    if FVisuals.ghost_kind ~= F.selected then
        if FVisuals.ghost ~= nil then F.destroy_entity(FVisuals.ghost) end
        FVisuals.ghost = F.spawn_entity("factory:entity", 0, 0)
        FVisuals.ghost_kind = F.selected
    end
    if FVisuals.ghost == nil then return end

    local proto = F.protos[F.selected]
    if proto == nil then return end
    set_texture(FVisuals.ghost, proto.graphics)

    local t = transform_of(FVisuals.ghost)
    if t == nil then return end

    if F.hover_cell == nil then
        t.position = Vector2f.new(-10000, -10000)
        return
    end

    local wx, wy = F.cell_center(F.hover_cell.x, F.hover_cell.y)
    if proto.flat then
        t.position = Vector2f.new(wx, wy)
    else
        t.position = Vector2f.new(wx + (proto.anchor_x or -32), wy + (proto.anchor_y or -64))
    end

    local valid = F.can_place(F.hover_cell.x, F.hover_cell.y) and F.in_reach(F.hover_cell.x, F.hover_cell.y)
    if valid then
        set_color(FVisuals.ghost, 160, 255, 160, 120)
    else
        set_color(FVisuals.ghost, 255, 120, 120, 120)
    end
end

function FVisuals.update(dt)
    FVisuals.t = FVisuals.t + dt
    update_machines(dt)
    update_particles(dt)
    update_ghost()
end

function FVisuals.fireworks()
    local cx = math.floor(Factory.control.width() * 0.5)
    local cy = math.floor(Factory.control.height() * 0.5)
    local wx, wy = F.cell_center(cx, cy)
    for _ = 1, 80 do
        local angle = math.random() * math.pi * 2
        local speed = 20 + math.random() * 60
        add_particle(math.random() < 0.5 and "glow_blue" or "glow_yellow",
            wx, wy,
            math.cos(angle) * speed, math.sin(angle) * speed,
            1.5 + math.random(), 0.6, 0.2, 255, 0, 40,
            255, 236, 170)
    end
end
