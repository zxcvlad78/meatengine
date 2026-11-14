local function spawn_player_for(pid)
    local e = nil
    if Network.is_server() then
        e = Network.spawn_entity("factorio:player", pid)
    else
        e = PackedEntity.spawn("factorio:player", F.registry())
    end
    if e ~= nil then
        F.set_player_spawn(e, pid)
    end
    return e
end

local function on_peer_joined(pid)
    Factory.control.on_peer_connected(pid)
    spawn_player_for(pid)
    F.notify("Player " .. tostring(pid) .. " joined")
end

local function on_peer_left(pid)
    F.notify("Player " .. tostring(pid) .. " left")
end

local function enter_world(w, r)
    F.active = true
    F.role = "single"
    if Network.is_server() then F.role = "server" elseif Network.is_client() then F.role = "client" end
    F.my_peer = Network.peer_id()
    F.inspect_cell = nil
    F.victory_shown = false

    F.refresh_catalog()

    local seed = 0
    if F.role ~= "client" then
        seed = (Network.now_ms and Network.now_ms() or 12345) % 2147483000 + 1
    end

    Factory.control.start(Network.is_server(), Network.is_client(), seed)
    F.seed = Factory.control.seed()

    if F.role ~= "client" then
        FWorldgen.build()
        FWorldgen.decor()
    end

    FUI.build(r)

    if F.role == "client" then
        Factory.control.request_sync()
    else
        spawn_player_for(F.my_peer)
        if Network.is_server() then
            Network.set_on_peer_connected(on_peer_joined)
            Network.set_on_peer_disconnected(on_peer_left)
        end
        F.notify("World seed " .. tostring(F.seed))
    end

    Factory.control.on_event("on_notice", function(event)
        F.notify(event.message or "")
    end)

    Factory.control.on_event("on_error", function(event)
        F.notify(event.message or "")
    end)
end

local function exit_world(w, r)
    F.active = false
    if Network.is_server() then
        Network.set_on_peer_connected(function() end)
        Network.set_on_peer_disconnected(function() end)
    end
    FVisuals.reset()
    FWorldgen.despawn()
    FInput.reset()
end

ScriptedStateRegistry.register("core:world", {
    on_enter = function(w, r) enter_world(w, r) end,
    on_exit = function(w, r) exit_world(w, r) end,
})
