Rpc.bind("core:play_audio", function(ctx, r)
    if Network.is_server() then
        local path = r:read_str()
        Rpc.invoke("core:echo_audio", path)
        SoundPlayer.play(ResourceLoader.load_SoundBuffer(path))
    end
end)

Rpc.bind("core:echo_audio", function(ctx, r)
    local path = r:read_str()
    SoundPlayer.play(ResourceLoader.load_SoundBuffer(path))
end)

local function send_chat_msg(text, sender)
    print("[Chat] " .. tostring(sender) .. ": " .. text)
end

Rpc.bind("core:chat_receive_msg", function(ctx, r)
    local text = r:read_str()
    local sender = r:read_u32()
    send_chat_msg(text, sender)
end)

Rpc.bind("core:chat", function(ctx, r)
    if Network.is_server() then
        local text = r:read_str()
        Rpc.invoke("core:chat_receive_msg", text, ctx.sender)
        send_chat_msg(text, ctx.sender)
    end
end)

