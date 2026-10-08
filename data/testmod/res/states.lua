ScriptedStateRegistry.register("testmod:menu", {
    on_enter = function(w, r) 
        local e = PackedEntity.spawn("core:ui:button", r)
        if r:has_Transform(e) then
            local transform = r:get_Transform(e)
            transform.position = Vector2f.new(350, 100)
        end
        if r:has_Label(e) then
            local label = r:get_Label(e)
            label.text = "Hello from testmod!!"
        end
    end,

    update = function(w, r, dt)

    end,
})
