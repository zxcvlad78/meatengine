FUI = {}

FUI.rects = {}
FUI.labels = {}
FUI.slots = {}
FUI.craft_buttons = {}
FUI.minimap_cells = {}
FUI.minimap_marker = nil
FUI.bar_segments = {}
FUI.toast_text = ""
FUI.toast_timer = 0
FUI.banner_title = ""
FUI.banner_subtitle = ""
FUI.banner_timer = 0
FUI.acc = 0
FUI.created = false

local UI = FConfig.ui

local function attach(r, e)
    if FUI.root ~= nil then
        Hierarchy.set_parent(r, e, FUI.root)
    end
    return e
end

local function panel(r, x, y, w, h, color)
    local e = r:create()
    r:add_Transform(e)
    local fr = r:add_FillRect(e)
    if fr ~= nil and fr.shape ~= nil then
        fr.shape.size = Vector2f.new(w, h)
    end
    local bg = r:add_BackgroundColor(e)
    bg.color = Color.new(color[1], color[2], color[3], color[4] or 220)
    local bc = r:add_BorderColor(e)
    bc.color = Color.new(70, 74, 84, 255)
    local bw = r:add_BorderWidth(e)
    bw.width = UiRect.all(Val.Px(1))
    local n = r:add_Node(e)
    n.position = PositionType.Absolute
    n.left = Val.Px(x)
    n.top = Val.Px(y)
    n.width = Val.Px(w)
    n.height = Val.Px(h)
    return attach(r, e)
end

local function label(r, x, y, text, size, color)
    local e = r:create()
    r:add_Transform(e)
    local l = r:add_Label(e)
    l.text = text
    l.character_size = size
    l.color = color or Color.new(230, 232, 240)
    local n = r:add_Node(e)
    n.position = PositionType.Absolute
    n.left = Val.Px(x)
    n.top = Val.Px(y)
    n.width = Val.Px(400)
    n.height = Val.Px(size + 6)
    return attach(r, e)
end

local function button(r, x, y, w, h, color, on_click)
    local e = panel(r, x, y, w, h, color)
    r:add_Interaction(e)
    r:add_FocusPolicy(e)
    ui.on_click(r, e, function(entity) on_click(entity) end)
    return e
end

local function set_label(r, entity, text)
    if entity == nil then return end
    if not r:valid(entity) then return end
    local l = r:get_Label(entity)
    if l ~= nil then l.text = text end
end

function FUI.build(r)
    FUI.root = r:create()
    r:add_Transform(FUI.root)
    r:add_Children(FUI.root)
    r:add_UIRoot(FUI.root)
    local rf = r:add_FlexStyle(FUI.root)
    rf.direction = FlexDirection.Column
    rf.align_items = AlignItems.FlexStart
    local rn = r:add_Node(FUI.root)
    rn.position = PositionType.Absolute
    rn.left = Val.Px(0)
    rn.top = Val.Px(0)
    rn.width = Val.Percent(100)
    rn.height = Val.Percent(100)

    FUI.rects = {}
    FUI.labels = {}
    FUI.slots = {}
    FUI.craft_buttons = {}
    FUI.minimap_cells = {}
    FUI.bar_segments = {}

    FUI.rects[#FUI.rects + 1] = { x = UI.inventory.x, y = UI.inventory.y, w = UI.inventory.w, h = UI.inventory.h }
    FUI.rects[#FUI.rects + 1] = { x = UI.production.x, y = UI.production.y, w = UI.production.w, h = UI.production.h }
    FUI.rects[#FUI.rects + 1] = { x = UI.crafting.x, y = UI.crafting.y, w = UI.crafting.w, h = UI.crafting.h }
    FUI.rects[#FUI.rects + 1] = { x = UI.inspect.x, y = UI.inspect.y, w = UI.inspect.w, h = UI.inspect.h }
    FUI.rects[#FUI.rects + 1] = { x = UI.minimap.x, y = UI.minimap.y, w = UI.minimap.w, h = UI.minimap.h }

    panel(r, UI.inventory.x, UI.inventory.y, UI.inventory.w, UI.inventory.h, { 18, 20, 26, 205 })
    FUI.labels.inventory_title = label(r, UI.inventory.x + 8, UI.inventory.y + 8, "INVENTORY", 15)

    panel(r, UI.production.x, UI.production.y, UI.production.w, UI.production.h, { 18, 20, 26, 205 })
    FUI.labels.production_title = label(r, UI.production.x + 8, UI.production.y + 8, "PRODUCTION", 15)
    FUI.labels.research = label(r, UI.production.x + 8, UI.production.y + 32, "", 11, Color.new(196, 214, 240))
    FUI.labels.research_next = label(r, UI.production.x + 8, UI.production.y + 50, "", 11, Color.new(142, 220, 168))
    FUI.labels.research_bar = label(r, UI.production.x + 8, UI.production.y + 68, "", 10, Color.new(150, 160, 175))
    FUI.labels.counters = label(r, UI.production.x + 8, UI.production.y + 86, "", 11, Color.new(205, 210, 220))

    panel(r, UI.crafting.x, UI.crafting.y, UI.crafting.w, UI.crafting.h, { 18, 20, 26, 205 })
    FUI.labels.crafting_title = label(r, UI.crafting.x + 8, UI.crafting.y + 8, "CRAFTING", 15)

    panel(r, UI.inspect.x, UI.inspect.y, UI.inspect.w, UI.inspect.h, { 18, 20, 26, 205 })
    FUI.labels.inspect_title = label(r, UI.inspect.x + 8, UI.inspect.y + 8, "INSPECT", 15)
    FUI.labels.inspect_body = label(r, UI.inspect.x + 8, UI.inspect.y + 32, "", 11, Color.new(200, 206, 216))

    local mm = UI.minimap
    panel(r, mm.x, mm.y, mm.w, mm.h, { 22, 26, 32, 235 })
    local cell = math.floor(mm.w / FConfig.minimap_cols)
    for j = 0, FConfig.minimap_rows - 1 do
        for i = 0, FConfig.minimap_cols - 1 do
            local e = r:create()
            r:add_Transform(e)
            local fr = r:add_FillRect(e)
            if fr ~= nil and fr.shape ~= nil then
                fr.shape.size = Vector2f.new(cell, cell)
            end
            local bg = r:add_BackgroundColor(e)
            bg.color = Color.new(40, 60, 40, 255)
            local n = r:add_Node(e)
            n.position = PositionType.Absolute
            n.left = Val.Px(mm.x + i * cell)
            n.top = Val.Px(mm.y + j * cell)
            n.width = Val.Px(cell)
            n.height = Val.Px(cell)
            attach(r, e)
            FUI.minimap_cells[(j * FConfig.minimap_cols + i) + 1] = e
        end
    end
    FUI.minimap_marker = r:create()
    r:add_Transform(FUI.minimap_marker)
    local marker_fr = r:add_FillRect(FUI.minimap_marker)
    if marker_fr ~= nil and marker_fr.shape ~= nil then
        marker_fr.shape.size = Vector2f.new(6, 6)
    end
    attach(r, FUI.minimap_marker)
    local bg = r:add_BackgroundColor(FUI.minimap_marker)
    bg.color = Color.new(255, 86, 86, 255)
    local n = r:add_Node(FUI.minimap_marker)
    n.position = PositionType.Absolute
    n.left = Val.Px(mm.x)
    n.top = Val.Px(mm.y)
    n.width = Val.Px(6)
    n.height = Val.Px(6)

    local order = F.build_order()
    for i, proto in ipairs(order) do
        local x = UI.hotbar.x + (i - 1) * UI.hotbar.step
        local e = button(r, x, UI.hotbar.y, UI.hotbar.w, UI.hotbar.h, { 70, 74, 86, 235 }, function()
            F.selected = proto.name
            F.notify("Selected " .. proto.label)
        end)
        FUI.slots[i] = e
        FUI.labels["slot_" .. i] = label(r, x + 4, UI.hotbar.y + 4, "", 10, Color.new(235, 238, 246))
        FUI.rects[#FUI.rects + 1] = { x = x, y = UI.hotbar.y, w = UI.hotbar.w, h = UI.hotbar.h }
    end

    local recipes = F.craft_order()
    for i, recipe in ipairs(recipes) do
        local y = UI.crafting.y + 32 + (i - 1) * 32
        local e = button(r, UI.crafting.x + 8, y, UI.crafting.w - 16, 28, { 60, 64, 76, 240 }, function()
            Factory.control.craft(recipe.name)
            F.play_sound("craft")
        end)
        FUI.craft_buttons[i] = e
        FUI.labels["craft_" .. i] = label(r, UI.crafting.x + 16, y + 6, "", 11, Color.new(240, 240, 245))
        FUI.rects[#FUI.rects + 1] = { x = UI.crafting.x + 8, y = y, w = UI.crafting.w - 16, h = 28 }
    end

    FUI.labels.status = label(r, UI.status.x, UI.status.y, "", 14)
    FUI.labels.toast = label(r, UI.toast.x, UI.toast.y, "", 15, Color.new(255, 226, 140))
    FUI.labels.banner = label(r, 340, 300, "", 30, Color.new(255, 240, 180))
    FUI.labels.banner_sub = label(r, 340, 350, "", 15, Color.new(230, 230, 240))
    FUI.labels.hints = label(r, UI.hints.x, UI.hints.y, "", 11, Color.new(170, 180, 195))

    FUI.created = true
end

function FUI.toast(text)
    FUI.toast_text = text
    FUI.toast_timer = 2.5
end

function FUI.banner(title, subtitle)
    FUI.banner_title = title
    FUI.banner_subtitle = subtitle
    FUI.banner_timer = 6.0
end

function FUI.mouse_over_ui(x, y)
    for _, rect in ipairs(FUI.rects) do
        if x >= rect.x and x <= rect.x + rect.w and y >= rect.y and y <= rect.y + rect.h then
            return true
        end
    end
    return false
end

local function cost_string(recipe)
    local parts = {}
    for _, pair in ipairs(recipe.ingredients or {}) do
        local item = F.items[pair.item]
        parts[#parts + 1] = tostring(pair.count) .. " " .. string.lower(item and item.label or pair.item)
    end
    if #parts == 0 then return "?" end
    return table.concat(parts, ", ")
end

local function refresh_inventory(r)
    local y = UI.inventory.y + 30
    local index = 1
    for name, item in pairs(F.items) do
        local entity = FUI.labels["inv_" .. name]
        if entity == nil then
            entity = label(r, UI.inventory.x + 8, y, "", 11, Color.new(205, 210, 220))
            FUI.labels["inv_" .. name] = entity
        end
        set_label(r, entity, string.format("%s: %d", item.label, F.inv_count(name)))
        y = y + 14
        index = index + 1
    end
end

local function refresh_crafting(r)
    local recipes = F.craft_order()
    for i, recipe in ipairs(recipes) do
        set_label(r, FUI.labels["craft_" .. i], recipe.label .. "  (" .. cost_string(recipe) .. ")")
    end
end

local function refresh_hotbar(r)
    local order = F.build_order()
    for i, proto in ipairs(order) do
        local count = F.inv_count(proto.name) or 0
        set_label(r, FUI.labels["slot_" .. i], i .. " " .. string.upper(proto.label) .. "\nx" .. count)
    end
end

local function refresh_research(r)
    local level = F.research.level or 0
    local total = F.research.units or 0
    set_label(r, FUI.labels.research, string.format("Research L%d  units %d", level, math.floor(total)))

    local active = F.technologies[F.research.active]
    if active ~= nil then
        set_label(r, FUI.labels.research_next,
            string.format("Next: %s (%d/%d)", active.label, math.floor(total), math.floor(active.unit_count)))
        set_label(r, FUI.labels.research_bar, string.rep("|", math.floor(total / math.max(1, active.unit_count) * FConfig.bar_segments)))
    else
        set_label(r, FUI.labels.research_next, "Research complete")
        set_label(r, FUI.labels.research_bar, string.rep("|", FConfig.bar_segments))
    end

    set_label(r, FUI.labels.counters, string.format("Plates: %d  Ore: %d  Science: %d  Built: %d",
        F.production.plates or 0, F.production.ore or 0, F.production.science or 0, F.stats.buildings or 0))
end

local function refresh_inspect(r)
    local info = F.inspect_info()
    if info == nil then
        set_label(r, FUI.labels.inspect_title, "INSPECT")
        set_label(r, FUI.labels.inspect_body, "Hover a building and press E")
        return
    end

    set_label(r, FUI.labels.inspect_title, string.upper(info.label or ""))
    local lines = {}
    for _, entry in ipairs(info.inv or {}) do
        local item = F.items[entry.item]
        lines[#lines + 1] = string.format("%s: %d", item and item.label or entry.item, entry.count)
    end
    if info.recipe ~= nil and info.recipe ~= "" then
        local recipe = F.recipes[info.recipe]
        lines[#lines + 1] = "Recipe: " .. (recipe and recipe.label or info.recipe)
    end
    if info.held ~= nil and info.held.item ~= nil and info.held.item ~= "" then
        local item = F.items[info.held.item]
        lines[#lines + 1] = "Holding: " .. (item and item.label or info.held.item)
    end
    set_label(r, FUI.labels.inspect_body, table.concat(lines, "\n"))
end

local function refresh_minimap(r)
    local width = Factory.control.width()
    local height = Factory.control.height()
    local mm = UI.minimap
    local cw = math.floor(mm.w / FConfig.minimap_cols)
    local ch = math.floor(mm.h / FConfig.minimap_rows)

    for j = 0, FConfig.minimap_rows - 1 do
        for i = 0, FConfig.minimap_cols - 1 do
            local sx = math.floor(i * width / FConfig.minimap_cols)
            local sy = math.floor(j * height / FConfig.minimap_rows)
            local entity = F.entity_at(sx, sy)
            local name = Factory.control.tile_at(sx, sy)
            local tile = F.tiles[name]
            local color = { 40, 60, 40, 255 }
            if entity ~= nil then
                color = { 255, 244, 170, 255 }
            elseif tile ~= nil then
                if tile.ore_item ~= nil and tile.ore_item ~= "" then
                    color = { 255, 190, 120, 255 }
                elseif tile.buildable == false then
                    color = { 60, 100, 160, 255 }
                else
                    color = { 74, 132, 62, 255 }
                end
            end
            local e = FUI.minimap_cells[(j * FConfig.minimap_cols + i) + 1]
            if e ~= nil and r:valid(e) then
                local bg = r:get_BackgroundColor(e)
                if bg ~= nil then
                    bg.color = Color.new(color[1], color[2], color[3], color[4])
                end
            end
        end
    end

    local cell = F.my_cell()
    if cell ~= nil and r:valid(FUI.minimap_marker) then
        local mx = math.floor(cell.x * FConfig.minimap_cols / math.max(1, width))
        local my = math.floor(cell.y * FConfig.minimap_rows / math.max(1, height))
        local n = r:get_Node(FUI.minimap_marker)
        if n ~= nil then
            n.left = Val.Px(mm.x + mx * cw)
            n.top = Val.Px(mm.y + my * ch)
        end
    end
end

local function refresh_status(r)
    local selected = F.protos[F.selected]
    local cell = F.my_cell()
    set_label(r, FUI.labels.status, string.format(
        "FACTORIO | role=%s seed=%d builds=%d | sel=%s dir=%s | cell=%s",
        F.role, F.seed or 0, F.stats.buildings or 0,
        selected and selected.label or "-", F.dir,
        cell and (cell.x .. "," .. cell.y) or "-"))
end

function FUI.update(dt)
    if not FUI.created then return end

    if FUI.toast_timer > 0 then
        FUI.toast_timer = FUI.toast_timer - dt
        if FUI.toast_timer <= 0 then FUI.toast_text = "" end
    end

    if FUI.banner_timer > 0 then
        FUI.banner_timer = FUI.banner_timer - dt
        if FUI.banner_timer <= 0 then
            FUI.banner_title = ""
            FUI.banner_subtitle = ""
        end
    end

    FUI.acc = FUI.acc + dt
    if FUI.acc < 0.2 then return end
    FUI.acc = 0

    local r = F.registry()
    if not r:valid(FUI.labels.status) then return end

    refresh_status(r)
    refresh_inventory(r)
    refresh_crafting(r)
    refresh_hotbar(r)
    refresh_research(r)
    refresh_inspect(r)
    refresh_minimap(r)

    set_label(r, FUI.labels.toast, FUI.toast_text)
    set_label(r, FUI.labels.banner, FUI.banner_title)
    set_label(r, FUI.labels.banner_sub, FUI.banner_subtitle)
    set_label(r, FUI.labels.hints,
        "LMB place / RMB mine|remove / 1-7 select / R rotate / E inspect / -+ zoom")
end
