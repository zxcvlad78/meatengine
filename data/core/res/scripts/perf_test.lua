local world = get_global_registry()

local test_texture = ResourceLoader.load_Texture("data/core/res/textures/test.png")

function test_spawn_entity()
    local entity = world:create()
    local transform = world:add_Transform(entity)
    local sprite = world:add_Sprite(entity)
    sprite.texture = test_texture
end

for i = 1, 1000 do
    test_spawn_entity()
end

