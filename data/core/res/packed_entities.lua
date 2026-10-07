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
-- in me::Console:
-- lua.run get_global_registry():get_Transform(PackedEntity.spawn("core:ui:button", get_global_registry())).position = Vector2f.new(300, 150)