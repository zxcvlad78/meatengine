PackedEntity.register("core:player", function(world)
    local e = world:create()
    world:add_Transform(e)
    world:add_Velocity(e)
    world:add_PlayerInput(e)
    local move_speed = world:add_MoveSpeed(e)
    move_speed.value = 100
    local cam = world:add_Camera(e)
    cam.zoom = 2
    return e
end)

PackedEntity.register("core:ui:button", function(world)
    local e = world:create()
    local transform = world:add_Transform(e)
    local fill_rect = world:add_FillRect(e)
    fill_rect.shape.size = Vector2f.new(250, 100)
    fill_rect.stylebox = ResourceLoader.load_StyleBox("data/core/res/styleboxes/default.json")
    local interactable = world:add_Interactable(e)
    local label = world:add_Label(e)
    label.font = ResourceLoader.load_Font("data/core/res/fonts/mainfont.ttf")
    label.text = "Core Button!!"
    return e
end)


-- example
-- in me::Console
-- lua.run MainLoop.get_registry():get_Transform(PackedEntity.spawn("core:ui:button", MainLoop.get_registry())).position = Vector2f.new(300, 150)