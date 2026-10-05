local wor;d = get_global_registry()

local sprite_count = 0

world:for_each_Sprite(function(entity, sp)
    sprite_count = sprite_count + 1
end)

print("Sprite count: ", sprite_count)
