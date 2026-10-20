PackedEntity.register("core:player", function(world)
    local e = world:create()
    world:add_Transform(e)
    world:add_Velocity(e)
    world:add_PlayerInput(e)
    local sp = world:add_Sprite(e)
    local sp_anim = world:add_SpriteAnimation(e)
    sp.texture = ResourceLoader.load_Texture("data/core/res/textures/pacan.png")
    local move_speed = world:add_MoveSpeed(e)
    move_speed.value = 100
    local cam = world:add_Camera(e)
    cam.zoom = 2
    return e
end)

PackedEntity.register("core:ui:base_node", function(world)
    local e = world:create()
    world:add_Transform(e)
    world:add_Children(e)
    world:add_Node(e)
    world:add_FlexStyle(e)
    return e
end)

PackedEntity.register("core:ui:label", function(world)
    local e = PackedEntity.spawn("core:ui:base_node", world)

    local lb = world:add_Label(e)
    lb.font = ResourceLoader.load_Font("data/core/res/fonts/mainfont.ttf")
    lb.text = "Label"

    return e
end)

PackedEntity.register("core:ui:title", function(world)
    local e = PackedEntity.spawn("core:ui:label", world)
    if e == nil then return e end

    local n = world:add_Node(e)
    n.height = Val.Px(64)

    local fs = world:add_FlexStyle(e)
    fs.align_self = AlignSelf.Center

    local c_lbl = world:get_Label(e)
    c_lbl.text = "Title"

    return e
end)

PackedEntity.register("core:ui:panel", function(world)
    local e = PackedEntity.spawn("core:ui:base_node", world)
    
    local n = world:add_Node(e)
    n.width = Val.Px(50)
    n.height = Val.Px(50)
    n.padding = UiRect.all(Val.Px(5))

    local fs = world:add_FlexStyle(e)
    fs.direction = FlexDirection.Column
    fs.justify = JustifyContent.Center
    fs.align_items = AlignItems.Center
    fs.row_gap = 5

    local fr = world:add_FillRect(e)
    fr.styled = true

    local bg = world:add_BackgroundColor(e)
    bg.color = Color.new(30, 30, 30, 230)

    local bc = world:add_BorderColor(e)
    bc.color = Color.new(0, 0, 0)

    local bw = world:add_BorderWidth(e)
    bw.width = UiRect.all(Val.Px(4))

    return e
end)

PackedEntity.register("core:ui:button", function(world)
    local e = PackedEntity.spawn("core:ui:base_node", world)

    local n = world:add_Node(e)
    n.width = Val.Px(250)
    n.height = Val.Px(50)
    n.padding = UiRect.all(Val.Px(4))

    local fs = world:add_FlexStyle(e)
    fs.direction = FlexDirection.Row
    fs.justify = JustifyContent.Center
    fs.align_items = AlignItems.Center

    local fr = world:add_FillRect(e)
    fr.styled = true

    local bg = world:add_BackgroundColor(e)
    bg.color = Color.new(120, 120, 120)


    local lbl = PackedEntity.spawn("core:ui:label", world)
    if lbl ~= nil then
        local c_lbl = world:get_Label(lbl)
        c_lbl.text = "Button"
        Hierarchy.set_parent(world, lbl, e)
    end

    world:add_Interaction(e)
    world:add_FocusPolicy(e)

    return e
end)