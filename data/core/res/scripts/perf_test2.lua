local world = get_global_registry()

local test_texture = ResourceLoader.load_Texture("res/textures/test2.png")

local sprite_count = 0

world:for_each_Sprite(function(entity, sp)
    sp.texture = test_texture
    sprite_count = sprite_count + 1
end)

print("Sprite count: ", sprite_count)
