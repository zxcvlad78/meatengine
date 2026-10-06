get_global_registry():for_each_TileMap(function(entity,tm)
	tm.set_tile(0, 1)
	print(tm.tiles)
end)
--print("enddd")

-- lua.run get_global_registry():for_each_TileMap(function(entity, tm) tm:set_tile(0, 0, 0) end)