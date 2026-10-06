PackedEntity.register("core:test_pacan", function(world)
    local e = world:create()
    local transform = world:add_Transform(e)

    local sprite = world:add_Sprite(e)
    sprite.texture = ResourceLoader.load_Texture("data/core/res/textures/pacan.png")
    
    return e
end)