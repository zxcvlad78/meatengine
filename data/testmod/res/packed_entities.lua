PackedEntity.register("testmod:player", function(world)
    local e = world:create()
    world:add_Transform(e)
    world:add_Velocity(e)
    world:add_PlayerInput(e)
    world:add_MoveSpeed(e)
    local cam = world:add_Camera(e)
    cam.zoom = 2
    return e
end)