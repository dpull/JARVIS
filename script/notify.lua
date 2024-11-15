local system = require "system"
local webclient = import("script/coroutine/webclient.lua")
local config = import("script/config.lua")

local notify_interval_ms = 1000 * 60 * 60
last_notify_time = last_notify_time or 0
last_notify_msg = last_notify_msg or ""

local function notify(msg)
    local now = system.get_time_ms()
    if last_notify_msg == msg then
        if now - last_notify_time < notify_interval_ms then
            return false
        end
    end

    last_notify_time = now
    last_notify_msg = msg

    webclient.request(config.notify_url, nil, msg)
    return true
end

function text(msg)
    local json_msg = string.format('{"msgtype": "text", "text": {"content": "%s"}}', msg)
    if notify(json_msg) then
        print("text", msg)
    end
end
