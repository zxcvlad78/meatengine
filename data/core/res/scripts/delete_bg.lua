local world = get_global_registry()

world:for_each_FillRect(function(entity, fr)
    if not fr.foreground then
		world:destroy(entity)
	end
end)

