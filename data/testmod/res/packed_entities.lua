PackedEntity.register("testmod:player", function(world)
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