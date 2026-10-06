local world = get_global_registry()

world:for_each_FillRect(function(entity, fr)
    if not fr.foreground then
		fr.shape.fill_color = Color.new(fr.shape.fill_color.r, fr.shape.fill_color.g, fr.shape.fill_color.b, 255)
	end
end)

get_global_registry():for_each_TileMap(function(entity,tm)tm.dirty=true end)