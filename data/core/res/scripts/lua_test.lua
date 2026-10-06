local world = get_global_registry()

local e = world:create()

world:add_Transform(e, Vector2.new(250, 250))
local fillrect = world:add_FillRect(e)
fillrect.shape.size = Vector2.new(175, 164)
fillrect.shape.fill_color = Color.new(255, 2, 2)

world:for_each_Label(function(entity, label)
    label.text = "Hello from lua!!"
end)

world:for_each_FillRect(function(entity, fr)
    fr.shape.fill_color = Color.new(255, 255, 255, 127)
    print("Fri")
end)