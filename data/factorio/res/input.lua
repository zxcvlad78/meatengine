FInput = {}

local prev_keys = {}
local prev_left = false
local prev_right = false
local mine_timer = 0
local camera_entity = nil

local function key_down(name) return Input.key_pressed(name) end

local function key_edge(name)
    local now = key_down(name)
    local was = prev_keys[name] or false
    prev_keys[name] = now
    return now and not was
end

local function camera()
    local r = F.registry()
    local found = nil
    r:foreach_Camera(function(e, c)
        if found == nil and Network.is_authority(e) and c:is_current() then
            found = c
        end
    end)
    if found == nil then
        r:foreach_Camera(function(e, c)
            if found == nil and Network.is_authority(e) then found = c end
        end)
    end
    return found
end

function FInput.reset()
    prev_keys = {}
    prev_left = false
    prev_right = false
    mine_timer = 0
end

function FInput.apply_zoom()
    local cam = camera()
    if cam == nil then return end
    cam.zoom = F.zoom
end

function FInput.select(index)
    local list = F.build_order()
    local proto = list[index]
    if proto == nil then return end
    F.selected = proto.name
    F.notify("Selected " .. proto.label)
end

function FInput.update_keys()
    for i = 1, 7 do
        if key_edge(tostring(i)) then FInput.select(i) end
    end
    if key_edge("r") then
        F.dir = F.dir % 4 + 1
    end
    if key_edge("minus") then
        F.zoom = math.max(1.0, F.zoom - 0.25)
    end
    if key_edge("equal") then
        F.zoom = math.min(4.5, F.zoom + 0.25)
    end
    if key_edge("e") and F.hover_cell ~= nil then
        local cell = F.key(F.hover_cell.x, F.hover_cell.y)
        if F.inspect_cell == cell then
            F.inspect_cell = nil
        elseif F.entities[cell] ~= nil then
            F.inspect_cell = cell
        end
    end
    if key_edge("escape") then
        F.inspect_cell = nil
    end
end

function FInput.update_mouse(screen, cx, cy, dt)
    local left = Input.mouse_just_pressed()
    local left_prev = prev_left
    prev_left = left

    local right_down = Input.mouse_right_down()
    local right_edge = right_down and not prev_right
    prev_right = right_down

    if FUI and FUI.mouse_over_ui and FUI.mouse_over_ui(screen.x, screen.y) then
        return
    end

    local function inside()
        return cx >= 0 and cy >= 0 and cx < Factory.control.width() and cy < Factory.control.height()
    end

    if left and not left_prev and inside() then
        if F.entity_at(cx, cy) ~= nil then
            Factory.control.rotate(cx, cy)
            F.play_sound("place")
        elseif F.selected ~= nil then
            Factory.control.build(F.selected, cx, cy, F.dir)
        end
    end

    if right_edge and inside() then
        if F.entity_at(cx, cy) ~= nil then
            Factory.control.remove(cx, cy)
        else
            Factory.control.mine(cx, cy)
        end
        mine_timer = 0
    elseif right_down and inside() then
        mine_timer = mine_timer + dt
        if mine_timer >= 0.35 then
            mine_timer = 0
            if F.entity_at(cx, cy) == nil then
                Factory.control.mine(cx, cy)
            end
        end
    else
        mine_timer = 0
    end
end

function FInput.update(dt)
    if not F.active then return end

    FInput.apply_zoom()
    FInput.update_keys()

    local screen = Input.mouse_screen()
    local cam = camera()
    local wx, wy = screen.x, screen.y
    if cam ~= nil then
        local world = cam:screen_to_world(screen)
        wx = world.x
        wy = world.y
    end

    local cx, cy = F.world_to_cell(wx, wy)
    F.hover_cell = { x = cx, y = cy }

    FInput.update_mouse(screen, cx, cy, dt)
end
