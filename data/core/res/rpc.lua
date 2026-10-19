Rpc.bind("core:play_audio", function(ctx, r)
    local path = r:ReadString()
    Rpc.invoke("core:echo_audio", path)
    if Network.is_server() then
        SoundPlayer.play(ResourceLoader.load_SoundBuffer(path))
    end
end)

Rpc.bind("core:echo_audio", function(ctx, r)
    local path = r:ReadString()
    SoundPlayer.play(ResourceLoader.load_SoundBuffer(path))
end)

local function send_chat_msg(text, sender)
    print("[Chat] " .. tostring(sender) .. ": " .. text)
end

Rpc.bind("core:chat", function(ctx, r)
    local text = r:ReadString()
    Rpc.invoke("core:echo_chat", text, ctx.sender)
    if Network.is_server() then
        send_chat_msg(text, ctx.sender)
    end
end)

Rpc.bind("core:echo_chat", function(ctx, r)
    local text = r:ReadString()
    local sender = r:ReadInt32()
    send_chat_msg(text, sender)
end)

