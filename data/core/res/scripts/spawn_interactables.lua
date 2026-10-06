local world = get_global_registry()

world:for_each_FillRect(function(entity, fr)
    if not world:has_Interactable(entity) then
		world:add_Interactable(entity)
		fr.dirty = true
	end
end)

