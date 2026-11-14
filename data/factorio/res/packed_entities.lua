local function register_sprite(id, z)
    PackedEntity.register(id, function(world)
        local e = world:create()
        world:add_Transform(e)
        local sp = world:add_Sprite(e)
        sp.center = true
        local zi = world:add_ZIndex(e)
        zi.value = z
        return e
    end)
end

register_sprite("factory:entity", 1)
register_sprite("factory:item", 2)
register_sprite("factory:overlay", 3)
register_sprite("factory:particle", 5)
register_sprite("factory:decor", -1)

PackedEntity.register("factorio:player", function(world)
    local e = world:create()
    world:add_Transform(e)
    world:add_Velocity(e)
    world:add_PlayerInput(e)
    world:add_Health(e)
    local speed = world:add_MoveSpeed(e)
    speed.value = 100
    local sp = world:add_Sprite(e)
    sp.center = false
    sp.offset = Vector2f.new(-16, -26)
    sp.texture = ResourceLoader.load_Texture("data/factorio/res/textures/character.png")
    local cam = world:add_Camera(e)
    cam.zoom = 2
    local z = world:add_ZIndex(e)
    z.value = 4
    return e
end)
