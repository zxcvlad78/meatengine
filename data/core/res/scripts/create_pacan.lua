local world = get_global_registry()

local pacan_texture = ResourceLoader.load_Texture("load_Texture")

local e = world:create()
local transform = world:add_Transform(e)

local sprite = world:add_Sprite(e)
sprite.texture = pacan_texture
