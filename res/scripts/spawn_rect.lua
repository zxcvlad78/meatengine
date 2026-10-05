local world = get_global_registry()

local e = world:create()

local transform = world:add_Transform(e)
transform.position.x = 350
transform.position.y = 250

local fillrect = world:add_FillRect(e)
fillrect.shape.size = Vector2f.new(78, 78)
fillrect.shape.fill_color = Color.new(255, 55, 15)
fillrect.stylebox = ResourceLoader.load_StyleBox("res/styleboxes/default.json")

print("rect pos: ", transform.position.x, transform.position.y)
print("rect color: ", fillrect.shape.fill_color.r, fillrect.shape.fill_color.g, fillrect.shape.fill_color.b)

local sb = fillrect.stylebox
if sb then
    print("foreground_color:", sb:get_color("foreground_color", Color.new(255,255,255)).a)
end