dofile(MOD_PATH .. "/data.lua")

local function load_part(name)
    dofile(MOD_PATH .. "/res/" .. name .. ".lua")
end

load_part("config")
load_part("packed_entities")
load_part("game")
load_part("worldgen")
load_part("visuals")
load_part("ui")
load_part("input")
load_part("states")

local sync_accum = 0

SystemRegistry.emplace(SystemPhase.Update, "factorio:update", 50, function()
    if not F.active then return end

    local dt = MainLoop.get_dt()

    Factory.control.tick(dt)
    Factory.control.client_tick(dt)

    if not F.world_ready and F.role == "client" and Factory.control.ready() then
        FWorldgen.build()
        FWorldgen.decor()
    end

    sync_accum = sync_accum + dt
    if sync_accum >= 0.1 then
        sync_accum = 0
        F.sync()
        FVisuals.sync_entities()
        FVisuals.sync_items()
    end

    FInput.update(dt)
    FVisuals.update(dt)
    FUI.update(dt)
end)
